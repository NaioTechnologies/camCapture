//==================================================================================================
//
//  Copyright(c)  2013 - 2015  Naïo Technologies
//
//  This program is free software: you can redistribute it and/or modify it under the terms of the
//  GNU General Public License as published by the Free Software Foundation, either version 3 of
//  the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
//  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along with This program.
//  If not, see <http://www.gnu.org/licenses/>.
//
//==================================================================================================

//==================================================================================================
// I N C L U D E   F I L E S

#include "BuildVersion.hpp"
#include "EntryPoint.hpp"

#include <IO/BlueFoxStereo.h>
#include <IO/JsonReader.hpp>
#include <IO/IOTiffWriter.hpp>
#include <IO/IOTiffReader.hpp>

#include "Control/CTPid.hpp"

#include <CLFileSystem.h>
#include <CLDate.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>


//==================================================================================================
// C O N S T A N T S   &   L O C A L   V A R I A B L E S

//==================================================================================================
// G L O B A L S

//==================================================================================================
// C O N S T R U C T O R (S) / D E S T R U C T O R   C O D E   S E C T I O N

//--------------------------------------------------------------------------------------------------
//
EntryPoint::EntryPoint()
	: parser_{ }
	, handler_{ }
	, signalHandler_{ }
	, signaled_{ }
{
	signalHandler_.attach_handler( ht::SignalHandler::Signal::Interrupt,
	                               std::bind( &EntryPoint::set_signal, this ) );

	signalHandler_.attach_handler( ht::SignalHandler::Signal::Termination,
	                               std::bind( &EntryPoint::set_signal, this ) );

	signalHandler_.start_watch();

	// Register program info
	std::string appName{ PROGRAM_NAME };
	appName.append( "-" );
	appName.append( PROGRAM_VERSION_STRING );

	// Set logger output name for the program
	HTLogger::SetExecModuleName( appName );

	auto f = std::bind( &EntryPoint::handle_parameters, this, std::placeholders::_1,
	                    std::placeholders::_2 );

	handler_.AddParamHandler( "-c", f );
	parser_.add_switch( "-c", "Calibrate stereo bench" );
}

//--------------------------------------------------------------------------------------------------
//
EntryPoint::~EntryPoint()
{
	signalHandler_.stop_watch();
}

//==================================================================================================
// M E T H O D S   C O D E   S E C T I O N


//--------------------------------------------------------------------------------------------------
//
void
EntryPoint::set_signal()
{
	signaled_ = true;
	ht::log_warning( "signal received" );
}

//--------------------------------------------------------------------------------------------------
//
bool
EntryPoint::is_signaled() const
{
	return signaled_;
}

//--------------------------------------------------------------------------------------------------
//
void
EntryPoint::print_header() const
{
	using namespace cl;
	print_line( "=============================================================================" );
	print_line( "" );
	print_line( "  ", PROGRAM_NAME, " version ", PROGRAM_VERSION_STRING, "  Copyright(c) 2013  ",
	            PROGRAM_OWNER );
	print_line( "  ", PROGRAM_DESCRIPTION );
	print_line( "" );
	print_line( "  This program is free software: you can redistribute it and/or modify" );
	print_line( "  it under the terms of the GNU General Public License as published by" );
	print_line( "  the Free Software Foundation, either version 3 of the License, or" );
	print_line( "  (at your option) any later version." );
	print_line( "" );
	print_line( "  This program is distributed in the hope that it will be useful," );
	print_line( "  but WITHOUT ANY WARRANTY; without even the implied warranty of" );
	print_line( "  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the" );
	print_line( "  GNU General Public License for more details." );
	print_line( "" );
	print_line( "=============================================================================" );
}

//--------------------------------------------------------------------------------------------------
//
bool
EntryPoint::handle_parameters( const std::string& paramName, const std::string& paramValue )
{
	cl::ignore( paramName, paramValue );
	return false;
}

//--------------------------------------------------------------------------------------------------
//
int32_t
EntryPoint::run( int32_t argc, const char** argv )
{
	int32_t res{ EXIT_SUCCESS };
	print_header();

	if( parser_.validate_cmd_line( argc, argv, &handler_ ) )
	{
		std::string resourceFolder{ "resources/" };

		io::JsonReader jsonReader;
		jsonReader.load( resourceFolder.append( "config.json" ) );
		Json::Value benchConfig( jsonReader.root()["stereobench"] );

		const uint32_t width = benchConfig.get( "width", 752 ).asUInt();
		const uint32_t height = benchConfig.get( "height", 480 ).asUInt();

		const uint32_t exposure = benchConfig.get( "exposure", 10000 ).asUInt();

		const uint8_t grayLevel = benchConfig.get( "average_gray_value", 50 ).asUInt();

		const bool autoexp = benchConfig.get( "auto_exposure", false ).asBool();
		const uint32_t minExposure = benchConfig.get( "exposure_min", 12 ).asUInt();
		const uint32_t maxExposure = benchConfig.get( "exposure_max", 20000 ).asUInt();

		const bool hdr = benchConfig.get( "hdr", false ).asBool();

		cv::Size size( static_cast<int32_t>(width), static_cast<int32_t>(height) );

		io::BlueFoxStereo stereoCapture;
		stereoCapture.start( ht::ColorSpace::Rgb, width, height, minExposure, maxExposure, hdr );

		std::string dateStr{ };
		CLDate date;
		date.GetDateAndTimeMIMEFormat( dateStr );
		cl::filesystem::folder_create( dateStr );

		cl::print_line( "Recording session in: ", dateStr );

		control::Pid pid;
		pid.set_pid_gains( 3, 0, 0 );
//1.32
		int32_t currentExposure{ maxExposure };

		int8_t pressed{ };
		while( !is_signaled() && pressed != 27 )
		{
			cm::BitmapPairEntryUniquePtr entry;
			stereoCapture.wait_entry( entry );

			stereoCapture.clear_entry_buffer();

			const std::string filePath =
				cl::filesystem::create_filespec( dateStr, std::to_string( entry->get_id() ),
				                                 io::tiff_file_extensions()[1] );


			//io::TiffWriter tiffWriter{ filePath };
			//tiffWriter
			//	.write_to_file( entry->bitmap_left(), entry->get_id(), entry->get_framerate() );
			//tiffWriter
			//	.write_to_file( entry->bitmap_right(), entry->get_id(), entry->get_framerate() );

			cv::Mat matL = cv::Mat( size, CV_8UC3 );
			cv::Mat matR = cv::Mat( size, CV_8UC3 );

			cv::Mat bgrL = cv::Mat( size, CV_8UC3 );
			cv::Mat bgrR = cv::Mat( size, CV_8UC3 );

			cv::Mat greyL = cv::Mat( size, CV_8UC1 );
			cv::Mat greyR = cv::Mat( size, CV_8UC1 );

			matL.data = entry->bitmap_left().data();
			matR.data = entry->bitmap_right().data();

			cv::cvtColor( matL, bgrL, CV_RGB2BGR );
			cv::cvtColor( matR, bgrR, CV_RGB2BGR );

			cv::cvtColor( matL, greyL, CV_RGB2GRAY );
			cv::cvtColor( matR, greyR, CV_RGB2GRAY );

			uint32_t greyLevelL{ };
			uint32_t greyLevelR{ };

			for( int32_t i = 0; i < greyL.rows; ++i )
			{
				for( int32_t j = 0; j < greyL.cols; ++j )
				{
					greyLevelL += greyL.at< uint8_t >( i, j );
					greyLevelR += greyR.at< uint8_t >( i, j );
				}
			}

			greyLevelL /= static_cast<uint32_t>(greyL.rows * greyL.cols);
			greyLevelR /= static_cast<uint32_t>(greyL.rows * greyL.cols);

			const int32_t greyLevel = (greyLevelL + greyLevelR) / 2;
			const int32_t greyLevelDiff = 50 - greyLevel;

			const int32_t correctedExposureError =
				static_cast<int32_t>(std::round(
					pid.compute_correction( static_cast<double>(greyLevelDiff) ) ));

			currentExposure += correctedExposureError;

			cl::print_line( greyLevelDiff, "; ", currentExposure );

			//if( currentExposure < 12 )
			//{
			//	currentExposure = 12;
			//}
			//else if( currentExposure > 20000 )
			//{
			//	currentExposure = 20000;
			//}

			stereoCapture.set_exposure( currentExposure );

			cv::Mat combine( std::max( bgrL.size().height, bgrR.size().height ),
			                 bgrL.size().width + bgrR.size().width, CV_8UC3 );

			cv::Mat left_roi( combine, cv::Rect( 0, 0, bgrL.size().width, bgrL.size().height ) );
			bgrR.copyTo( left_roi );

			cv::Mat right_roi( combine, cv::Rect( bgrL.size().width, 0, bgrR.size().width,
			                                      bgrR.size().height ) );
			bgrL.copyTo( right_roi );

			cv::imshow( "images", combine );

			pressed = static_cast<int8_t>(cv::waitKey( 10 ));
		}
		stereoCapture.stop();
	}

	return res;
}


//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//----------------------------------------- Main Function ------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

int32_t
main( int32_t argc, const char** argv )
{
	int32_t ret{ EXIT_SUCCESS };

	try
	{
		ret = EntryPoint().run( argc, argv );
	}
	catch( const cl::SystemError& e )
	{
		ht::log_fatal( "System Error: ", e.what(), e.where() );
		throw;
	}
	catch( const cl::BaseException& e )
	{
		ht::log_fatal( "BaseException caught: ", e.what() );
		throw;
	}
	catch( const std::exception& e )
	{
		ht::log_fatal( "std::exception caught: ", e.what() );
		throw;
	}
	catch( ... )
	{
		ht::log_fatal( "Caught an exception of an undetermined type" );
		throw;
	}

	return ret;
}
