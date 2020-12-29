// This file is part of comma, a generic and flexible library
// Copyright (c) 2011 The University of Sydney
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. Neither the name of the University of Sydney nor the
//    names of its contributors may be used to endorse or promote products
//    derived from this software without specific prior written permission.
//
// NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
// GRANTED BY THIS LICENSE.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
// HOLDERS AND CONTRIBUTORS \"AS IS\" AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
// BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
// IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <termios.h>
#include <iostream>
#include <boost/program_options.hpp>
#include <boost/scoped_ptr.hpp>
#include "../../application/signal_flag.h"
#include "../select.h"

int main( int argc, char** argv )
{
    try
    {
        double period;
        boost::program_options::options_description description( "options" );
        description.add_options()
            ( "help,h", "display help message" )
            ( "heartbeat,b", "output byte with value 0x00 when key pressed" )
            ( "period,t", boost::program_options::value< double >( &period )->default_value( 0.1, "0.1" ), "period in seconds between heartbeats" );

        boost::program_options::variables_map vm;
        boost::program_options::store( boost::program_options::parse_command_line( argc, argv, description), vm );
        boost::program_options::notify( vm );

        if ( vm.count( "help" ) )
        {
            std::cerr << "take key presses, output key numeric values to stdout" << std::endl;
            std::cerr << "usage: io-console [<options>]" << std::endl;
            std::cerr << description << std::endl;
            return 1;
        }

        boost::scoped_ptr< comma::io::select > select;
        unsigned int seconds = 0;
        unsigned int nanoseconds = 0;
        if ( vm.count( "heartbeat" ) )
        {
            select.reset( new comma::io::select() );
            select->read().add( 0 );
            seconds = std::floor( period );
            nanoseconds = std::floor( double( seconds == 0 ? period : std::fmod( period, seconds ) ) * 1e9 );
        }
        
        // remove echo from the console
        struct termios oldTerm;
        struct termios newTerm;
        ::tcgetattr( STDIN_FILENO, &oldTerm );
        newTerm = oldTerm;
        newTerm.c_lflag &= ~( ICANON | ECHO );
        ::tcsetattr( STDIN_FILENO, TCSANOW, &newTerm );
        
        comma::signal_flag signal;
        while( !signal && std::cout.good() && !std::cout.bad() )
        {
            char c = 0x00;
            if( !select || select->wait( seconds, nanoseconds ) )
            {
                std::cin.read( &c, 1 );
                if( std::cin.gcount() <= 0 ) { break; }
            }
            std::cout.write( &c, 1 );
            std::cout.flush();
        }
        ::tcsetattr( STDIN_FILENO, TCSANOW, &oldTerm ); // restore the console
        return 0;
    }
    catch( std::exception& ex ) { std::cerr << argv[0] << ": " << ex.what() << std::endl; }
    catch( ... ) { std::cerr << argv[0] << ": unknown exception" << std::endl; }
    return 1;
}
