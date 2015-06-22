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

#include <IO/IOBlueFoxStereo.hpp>
#include <IO/JsonReader.hpp>
#include "IO/IOTiffWriter.hpp"
#include "IO/IOBufferWriter.hpp"
#include "IO/IOFileWriter.hpp"
#include "Control/CTPid.hpp"
#include "VisionModule/VMConversion.hpp"
#include "VisionModule/VMStatistics.hpp"

#include "HTUtility.h"
#include "HTBitmap.hpp"
#include "CLFileSystem.h"

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
	: signaled_{ }
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
		const uint32_t greyLevelTarget = benchConfig.get( "average_gray_value", 50 ).asUInt();
		const bool autoexp = benchConfig.get( "auto_exposure", false ).asBool();
		const uint32_t minExposure = benchConfig.get( "exposure_min", 12 ).asUInt();
		const uint32_t maxExposure = benchConfig.get( "exposure_max", 20000 ).asUInt();

		cl::ignore( exposure, autoexp );

		const bool hdr = benchConfig.get( "hdr", false ).asBool();

		cv::Size size( static_cast<int32_t>(width), static_cast<int32_t>(height) );

		io::BlueFoxStereo stereoBench;

		io::CameraInfo infoL = stereoBench.get_camera_info( io::BlueFoxStereo::Position::Left );
		io::CameraInfo infoR = stereoBench.get_camera_info( io::BlueFoxStereo::Position::Right );
		io::Intrinsics intrinsicsL, intrinsicsR;
		io::Extrinsics extrinsics;
		uint32_t serialNumL = cl::str_to_uint32( infoL.serialNum );
		uint32_t serialNumR = cl::str_to_uint32( infoR.serialNum );
		uint8_t mode{ 0x01 };

		stereoBench.read_bench_params( intrinsicsL, intrinsicsR, extrinsics );

		std::string dateStr{ };
		CLDate date;
		date.GetDateAndTimeMIMEFormat( dateStr );
		cl::filesystem::folder_create( dateStr );

		cl::print_line( "Recording session in: ", dateStr );

		size_t allParamSize = sizeof( mode ) + sizeof( serialNumL ) + sizeof( io::Intrinsics ) +
		                      sizeof( serialNumR ) + sizeof( io::Intrinsics ) +
		                      sizeof( io::Extrinsics );

		io::BufferWriter bufferWriter( allParamSize );

		bufferWriter.write( mode );

		bufferWriter.write( serialNumL );
		io::IntrinsicsArray intrinsicsArrayL = intrinsicsL.to_array();
		bufferWriter.write_array( intrinsicsArrayL.data(), intrinsicsArrayL.size() );

		bufferWriter.write( serialNumR );
		io::IntrinsicsArray intrinsicsArrayR = intrinsicsR.to_array();
		bufferWriter.write_array( intrinsicsArrayR.data(), intrinsicsArrayR.size() );

		io::ExtrinsicsArray extrinsicsArray = extrinsics.to_array();
		bufferWriter.write_array( extrinsicsArray.data(), extrinsicsArray.size() );

		const std::string filePathP = cl::filesystem::create_filespec( dateStr, "capture", "bin" );

		io::write_buffer_to_file( filePathP, bufferWriter.get_buffer() );

		stereoBench.start( ht::ColorSpace::RGB, width, height, minExposure, maxExposure, hdr );

		control::Pid pid;
		pid.set_pid_gains( 3, 0, 0 );

		int32_t currentExposure{ static_cast<int32_t>(maxExposure) };

		size_t nbr{ };
		int8_t pressed{ };
		while( !is_signaled() && pressed != 27 )
		{
			cm::BitmapPairEntryUniquePtr entry;
			stereoBench.wait_entry( entry );

			stereoBench.clear_entry_buffer();

			ht::BitmapUPtr grayL = ht::unique_bitmap( entry->bitmap_left().width(),
			                                          entry->bitmap_left().height(),
			                                          entry->bitmap_left().bit_depth(),
			                                          ht::ColorSpace::Grayscale );

			ht::BitmapUPtr grayR = ht::unique_bitmap( entry->bitmap_left().width(),
			                                          entry->bitmap_left().height(),
			                                          entry->bitmap_left().bit_depth(),
			                                          ht::ColorSpace::Grayscale );

			vm::rgb_to_bgr( entry->bitmap_left(), *grayL );
			vm::rgb_to_bgr( entry->bitmap_right(), *grayR );

			ht::BitmapUPtr bgrL = ht::unique_bitmap( entry->bitmap_left().width(),
			                                         entry->bitmap_left().height(),
			                                         entry->bitmap_left().bit_depth(),
			                                         ht::ColorSpace::BGR );

			ht::BitmapUPtr bgrR = ht::unique_bitmap( entry->bitmap_left().width(),
			                                         entry->bitmap_left().height(),
			                                         entry->bitmap_left().bit_depth(),
			                                         ht::ColorSpace::BGR );

			vm::rgb_to_bgr( entry->bitmap_left(), *bgrL );
			vm::rgb_to_bgr( entry->bitmap_right(), *bgrR );

			double greyLevelL{ };
			double greyLevelR{ };

			vm::mean( *grayL, greyLevelL );
			vm::mean( *grayR, greyLevelR );

			const double greyLevel = ( greyLevelL + greyLevelR ) / 2;
			const int32_t greyLevelDiff = static_cast<int32_t>(greyLevelTarget) - greyLevel;

			const int32_t correctedExposureError =
				static_cast<int32_t>(std::round(
					pid.compute_correction( static_cast<double>(greyLevelDiff) ) ));

			currentExposure += correctedExposureError;

			if( currentExposure < 12 )
			{
				currentExposure = 12;
			}
			else if( currentExposure > 20000 )
			{
				currentExposure = 20000;
			}

			stereoBench.set_exposure( static_cast<uint32_t>(currentExposure) );

			cv::Mat matL = cv::Mat( size, CV_8UC3 );
			cv::Mat matR = cv::Mat( size, CV_8UC3 );

			matL.data = entry->bitmap_left().data();
			matR.data = entry->bitmap_right().data();

			cv::Mat combine( std::max( matL.size().height, matR.size().height ),
			                 matL.size().width + matR.size().width, CV_8UC3 );

			cv::Mat left_roi( combine, cv::Rect( 0, 0, matL.size().width, matL.size().height ) );
			matL.copyTo( left_roi );

			cv::Mat right_roi( combine, cv::Rect( matL.size().width, 0, matR.size().width,
			                                      matR.size().height ) );
			matR.copyTo( right_roi );

			cv::imshow( "images", combine );

			pressed = static_cast<int8_t>( cv::waitKey( 1 ) );
		}
		stereoBench.stop();
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
