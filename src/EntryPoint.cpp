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

#include "BaseFilters/BFDemosaicingFilter.hpp"
#include "BaseFilters/BFExposureFilter.hpp"
#include "BaseFilters/BFRectificationFilter.hpp"

#include "IO/IOFileWriter.hpp"
#include "IO/IOStereoRigCalibration.hpp"
#include <IO/IOJsonReader.hpp>
#include "IO/IOTiffWriter.hpp"
#include "IO/IOBufferWriter.hpp"

#include "HTUtility.h"
#include "HTBitmap.hpp"
#include "CLFileSystem.h"

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
		const io::JsonElement benchConfig = jsonReader.get_root().get( "stereobench" );

		io::BlueFox::Params blueFoxParams;
		blueFoxParams.colorSpace = ht::ColorSpace::RAW;
		blueFoxParams.width = 752;
		blueFoxParams.height = 480;
		blueFoxParams.exposure = 20000;
		blueFoxParams.autoExposure = false;
		blueFoxParams.exposureMax = 20000;
		blueFoxParams.exposureMin = 12;
		blueFoxParams.hdrEnabled = true;
		blueFoxParams.periodInUs = 45000;

		im::BlueFoxImporterUPtr importer = im::unique_bluefox_importer( blueFoxParams );

		io::StereoRigCalibration calibrationParams;
		calibrationParams.read_from_stereo_rig( *importer );

		io::Intrinsics intrinsicsL = calibrationParams.get_intrinsics_left();
		io::Intrinsics intrinsicsR = calibrationParams.get_intrinsics_left();
		io::Extrinsics extrinsics = calibrationParams.get_extrinsics();

		uint32_t serialNumL = cl::str_to_uint32( importer->get_camera_left().get_serial_number() );
		uint32_t serialNumR = cl::str_to_uint32( importer->get_camera_right().get_serial_number() );
		uint8_t mode{ 0x00 };

		std::string dateStr{ };
		cl::Date date;
		date.get_date_and_time_mime( dateStr );
		cl::filesystem::folder_create( dateStr );

		cl::print_line( "Recording session in: ", dateStr );

		size_t allParamSize = sizeof( mode ) + sizeof( serialNumL ) + sizeof( io::Intrinsics ) +
			sizeof( serialNumR ) + sizeof( io::Intrinsics ) +
			sizeof( io::Extrinsics );

		io::BufferWriter bufferWriter( allParamSize );

		bufferWriter.write( mode );

		//bufferWriter.write( serialNumL );
		//io::IntrinsicsArray intrinsicsArrayL = intrinsicsL.to_array();
		//bufferWriter.write_array( intrinsicsArrayL.data(), intrinsicsArrayL.size() );
		//
		//bufferWriter.write( serialNumR );
		//io::IntrinsicsArray intrinsicsArrayR = intrinsicsR.to_array();
		//bufferWriter.write_array( intrinsicsArrayR.data(), intrinsicsArrayR.size() );
		//
		//io::ExtrinsicsArray extrinsicsArray = extrinsics.to_array();
		//bufferWriter.write_array( extrinsicsArray.data(), extrinsicsArray.size() );

		const std::string filePathP = cl::filesystem::create_filespec( dateStr, "capture", "bin" );

		io::write_buffer_to_file( filePathP, bufferWriter.get_buffer() );

		vm::Size size{ blueFoxParams.width, blueFoxParams.height };
		cl::Rect2u32 roi{ 0, 0, size.width(), size.height() };
		co::OutputMetrics om{ size, roi };

		FileOutput output( dateStr );
		this->add_output( output );

		bf::DemosaicingFilter demosaicingFilter;
		demosaicingFilter.prepare_filter( om );
		this->add_output( demosaicingFilter );

		bf::RectificationFilter rectFilter( calibrationParams );
		rectFilter.prepare_filter( om );
		demosaicingFilter.add_output( rectFilter );

		bf::ExposureFilter exposureFilter;
		exposureFilter.prepare_filter( om );
		rectFilter.add_output( exposureFilter );

		cm::BitmapCache bitmapCache;
		importer->start_async_read( bitmapCache );

		size_t nbr{ };
		int8_t pressed{ };

		while( !is_signaled() && pressed != 27 )
		{
			if( bitmapCache.wait_for_new_entry( 0 ) )
			{
				cm::BitmapPairEntrySPtr entry{ };
				bool status = bitmapCache.pop_newest_entry( entry );
				if( status )
				{
					co::OutputMetrics om{ size, roi };
					co::OutputResult result{ om };
					co::ParamContext ctx( bitmapCache );

					result.add_cache_entries( entry->get_cache_id(), entry );
					result.start_benchmark();

					if( compute_result( ctx, result ) )
					{
						result.stop_benchmark();
						cl::print_line_sp( "VisualCortex successfully updated" );
						result.print_benchmark( "" );
						cl::print_line();

						importer
							->set_exposure_overshoot( exposureFilter.get_greylevel_overshoot() );
					}
					else
					{
						result.stop_benchmark();
						cl::print_line_sp( "VisualCortex update failed" );
						result.print_benchmark( "" );
						cl::print_line();
					}
				}
			}
		}
		//	cm::BitmapPairEntryUPtr entry;
		//	if( stereoBench.wait_entry( entry ) )
		//	{
		//		stereoBench.clear_entry_buffer();
		//
		//		ht::BitmapUPtr grayL = ht::unique_bitmap( entry->bitmap_left().width(),
		//												  entry->bitmap_left().height(),
		//												  entry->bitmap_left().bit_depth(),
		//												  ht::ColorSpace::Grayscale );
		//
		//		ht::BitmapUPtr grayR = ht::unique_bitmap( entry->bitmap_left().width(),
		//												  entry->bitmap_left().height(),
		//												  entry->bitmap_left().bit_depth(),
		//												  ht::ColorSpace::Grayscale );
		//
		//		std::string filePath = cl::filesystem::create_filespec(
		//			dateStr, std::to_string( entry->get_id() ), io::tiff_file_extensions()[1] );
		//
		//		//cl::print_line( filePath );
		//		//io::TiffWriter tiffWriter{ filePath };
		//		//tiffWriter.write_to_file( entry->bitmap_left(), entry->get_id(),
		//		//						  entry->get_framerate() );
		//		//tiffWriter.write_to_file( entry->bitmap_right(), entry->get_id(),
		//		//						  entry->get_framerate() );
		//
		//		vm::rgb_to_grey( entry->bitmap_left(), *grayL );
		//		vm::rgb_to_grey( entry->bitmap_right(), *grayR );
		//
		//		double greyLevelL = vm::mean( *grayL );
		//		double greyLevelR = vm::mean( *grayR );
		//
		//		const double greyLevel = (greyLevelL + greyLevelR) / 2;
		//		const int32_t greyLevelDiff = 70 - greyLevel;
		//
		//		const int32_t correctedExposureError =
		//			static_cast<int32_t>(std::round(
		//				pid.compute_correction( static_cast<double>(greyLevelDiff) ) ));
		//
		//		currentExposure += correctedExposureError;
		//
		//		if( currentExposure < static_cast<int32_t>(stereoRigParams.exposureMin) )
		//		{
		//			currentExposure = static_cast<int32_t>(stereoRigParams.exposureMin);
		//		}
		//		else if( currentExposure > static_cast<int32_t>(stereoRigParams.exposureMax) )
		//		{
		//			currentExposure = static_cast<int32_t>(stereoRigParams.exposureMax);
		//		}
		//
		//		stereoBench.set_exposure( static_cast<uint32_t>(currentExposure) );
		//
		//		cv::Mat matL = cv::Mat( size, CV_8UC3 );
		//		cv::Mat matR = cv::Mat( size, CV_8UC3 );
		//
		//		matL.data = entry->bitmap_left().data();
		//		matR.data = entry->bitmap_right().data();
		//
		//		cv::Mat combine( std::max( matL.size().height, matR.size().height ),
		//						 matL.size().width + matR.size().width, CV_8UC3 );
		//
		//		cv::Mat
		//			left_roi( combine, cv::Rect( 0, 0, matL.size().width, matL.size().height ) );
		//		matL.copyTo( left_roi );
		//
		//		cv::Mat right_roi( combine, cv::Rect( matL.size().width, 0, matR.size().width,
		//											  matR.size().height ) );
		//		matR.copyTo( right_roi );
		//
		//		cv::imshow( "images", combine );
		//
		//		pressed = static_cast<int8_t>( cv::waitKey( 10 ) );
		//	}
		//}
		importer->stop_async_read();
		importer->close();
	}

	return res;
}

//--------------------------------------------------------------------------------------------------
//
bool
EntryPoint::compute_result( co::ParamContext& context, const co::OutputResult& result )
{
	for( auto& iter : get_output_list() )
	{
		if( iter )
		{
			if( !iter->compute_result( context, result ) )
			{
				return false;
			}
		}
	}
	return true;
}

//--------------------------------------------------------------------------------------------------
//
bool
EntryPoint::query_output_metrics( co::OutputMetrics& om )
{
	cl::ignore( om );
	return true;
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
