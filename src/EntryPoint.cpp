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
#include "IO/IOBlueFoxStereoCalib.hpp"
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

		std::string dateStr{ };
		cl::Date date;
		date.get_date_and_time_mime( dateStr );
		cl::filesystem::folder_create( dateStr );

		im::BlueFoxStereoImporterUPtr
			importer = im::unique_bluefox_stereo_importer( blueFoxParams );

		io::BlueFoxStereoCalib calibrationParams;
		calibrationParams.load_from_stereo_rig( *importer );
		calibrationParams.save_to_file( dateStr, "capture" );

		vm::Size size{ blueFoxParams.width, blueFoxParams.height };
		cl::Rect2u32 roi{ 0, 0, size.width(), size.height() };
		co::OutputMetrics om{ size, roi };

		FileOutput output( dateStr );
		this->add_output( output );

		bf::DemosaicingFilter demosaicingFilter;
		demosaicingFilter.prepare_filter( om );
		this->add_output( demosaicingFilter );

		bf::ExposureFilter exposureFilter;
		exposureFilter.prepare_filter( om );
		demosaicingFilter.add_output( exposureFilter );

		cm::BitmapCache bitmapCache;
		importer->open( "" );
		importer->start_async_read( bitmapCache );

		int8_t pressed{ };

		while( !is_signaled() && pressed != 27 )
		{
			if( bitmapCache.wait_for_new_entry( 0 ) )
			{
				cm::BitmapPairEntrySPtr entry{ };
				bool status = bitmapCache.pop_newest_entry( entry );
				if( status )
				{
					co::OutputResult result{ om };
					co::ParamContext ctx( bitmapCache );

					result.add_cache_entries( entry->get_cache_id(), entry );
					result.start_benchmark();

					if( compute_result( ctx, result ) )
					{
						result.stop_benchmark();
						//cl::print_line_sp( "VisualCortex successfully updated" );
						//result.print_benchmark( "" );
						//cl::print_line();

						importer->set_exposure_overshoot( exposureFilter.get_greylevel_diff() );
					}
					else
					{
						//result.stop_benchmark();
						//cl::print_line_sp( "VisualCortex update failed" );
						//result.print_benchmark( "" );
						//cl::print_line();
					}
				}
			}
		}

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
//
bool
EntryPoint::query_output_format( co::OutputFormat& of )
{
	cl::ignore( of );
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
		cl::print_line( "System Error: ", e.what(), e.where() );
		throw;
	}
	catch( const cl::BaseException& e )
	{
		cl::print_line( "BaseException caught: ", e.what() );
		throw;
	}
	catch( const std::exception& e )
	{
		cl::print_line( "std::exception caught: ", e.what() );
		throw;
	}
	catch( ... )
	{
		cl::print_line( "Caught an exception of an undetermined type" );
		throw;
	}

	return ret;
}
