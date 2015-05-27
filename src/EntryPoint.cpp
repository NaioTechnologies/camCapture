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

#include <CLFileSystem.h>
#include <CLDate.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>


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

		uint32_t width = benchConfig.get( "width", 752 ).asUInt();
		uint32_t height = benchConfig.get( "height", 480 ).asUInt();

		uint32_t exposure = benchConfig.get( "exposure", 10000 ).asUInt();

		bool autoexp = benchConfig.get( "auto_exposure", false ).asBool();
		uint32_t lowerLimit = benchConfig.get( "exposure_min", 50 ).asUInt();
		uint32_t upperLimit = benchConfig.get( "exposure_max", 1000 ).asUInt();

		bool hdr = benchConfig.get( "hdr", false ).asBool();

		cv::Size size( static_cast<int32_t>(width), static_cast<int32_t>(height) );

		cl::ignore( exposure, autoexp );

		io::BlueFoxStereo stereoCapture;

		if( autoexp )
		{
			stereoCapture.start( ht::ColorSpace::Rgb, width, height, 40000, lowerLimit, upperLimit,
			                     hdr );
		}
		else
		{
			stereoCapture.start( ht::ColorSpace::Rgb, width, height, 40000, exposure, hdr );
		}

		std::string dateStr{ };
		CLDate date;
		date.GetDateAndTimeMIMEFormat( dateStr );
		cl::filesystem::folder_create( dateStr );

		cl::print_line( "Recording session in: ", dateStr );

		//io::TiffWriter tiffWriter;

		uint64_t imageCount{ };
		int8_t pressed{ };
		while( !is_signaled() && pressed != 27 )
		{
			io::StereoEntryUniquePtr entry;
			stereoCapture.wait_entry( entry );

			stereoCapture.clear_entry_buffer();

			cv::Mat3b matL = cv::Mat3b::zeros( size );
			cv::Mat3b matR = cv::Mat3b::zeros( size );

			matL.data = entry->bitmap_left().data();
			matR.data = entry->bitmap_right().data();

			std::string file_path_l{ dateStr };
			file_path_l.append( "/" );
			file_path_l.append( std::to_string( imageCount ) );
			file_path_l.append( "_l.tiff" );

			std::string file_path_r{ dateStr };
			file_path_r.append( "/" );
			file_path_r.append( std::to_string( imageCount ) );
			file_path_r.append( "_r.tiff" );

			//tiffWriter.write_to_file( entry->bitmap_left(), file_path_l );

			cv::imwrite( file_path_l, matL );
			cv::imwrite( file_path_r, matR );

			cv::imshow( "left", matL );
			cv::imshow( "right", matR );

			pressed = static_cast<int8_t>(cv::waitKey( 10 ));

			++imageCount;
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
	int32_t ret{ };

	try
	{
		EntryPoint ep;
		ret = ep.run( argc, argv );
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
