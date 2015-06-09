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

#ifndef ENTRYPOINT_HPP
#define ENTRYPOINT_HPP

//==================================================================================================
// I N C L U D E   F I L E S

#include "HTCmdLineParser.h"
#include "HTLogger.h"
#include "HTSignalHandler.h"
#include <string>


//==================================================================================================
// F O R W A R D   D E C L A R A T I O N S

//==================================================================================================
// C O N S T A N T S

//==================================================================================================
// C L A S S E S

class EntryPoint
{
//--Methods-----------------------------------------------------------------------------------------
public:
	EntryPoint();

	~EntryPoint();

	void set_signal();

	bool is_signaled() const;

	/// Outputs a header for the program on the standard output stream.
	void print_header() const;

	bool handle_parameters( const std::string& option, const std::string& type );

	/// Main entry point of the application.
	int32_t run( int32_t argc, const char** argv );

//--Data members------------------------------------------------------------------------------------
private:
	/// Command line parser, logger and debugger for the application
	HTCmdLineParser parser_;
	HTCmdLineParser::Visitor handler_;

	ht::SignalHandler signalHandler_;

	volatile bool signaled_;
};


//==================================================================================================
// I N L I N E   F U N C T I O N S   C O D E   S E C T I O N

#endif  // ENTRYPOINT_HPP
