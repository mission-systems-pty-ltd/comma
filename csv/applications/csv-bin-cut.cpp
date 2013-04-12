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
// 3. All advertising materials mentioning features or use of this software
//    must display the following acknowledgement:
//    This product includes software developed by the The University of Sydney.
// 4. Neither the name of the The University of Sydney nor the
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


/// @author vsevolod vlaskine

#ifdef WIN32
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <deque>
#include <iostream>
#include <boost/lexical_cast.hpp>
#include <comma/application/contact_info.h>
#include <comma/application/command_line_options.h>
#include <comma/application/signal_flag.h>
#include <comma/base/exception.h>
#include <comma/csv/format.h>
#include <comma/string/string.h>

using namespace comma;

static void usage()
{
    std::cerr << std::endl;
    std::cerr << "simplified, but similar as Linux cut utility, but for \"binary csv\"" << std::endl;
    std::cerr << std::endl;
    std::cerr << "usage: cat blah.bin | csv-bin-cut <format> --fields=<fields>" << std::endl;
    std::cerr << "    <fields>: field numbers, starting from 1 (to keep" << std::endl;
    std::cerr << "              consistent with the standard cut utility)" << std::endl;
    std::cerr << std::endl;
    std::cerr << csv::format::usage() << std::endl;
    std::cerr << std::endl;
    std::cerr << comma::contact_info << std::endl;
    std::cerr << std::endl;
    exit( -1 );
}

int main( int ac, char** av )
{
    #ifdef WIN32
    _setmode( _fileno( stdin ), _O_BINARY ); /// @todo move to a library
    _setmode( _fileno( stdout ), _O_BINARY ); /// @todo move to a library
    #endif
    try
    {        
        signal_flag shutdownFlag;
        command_line_options options( ac, av );
        if( ac < 2 || options.exists( "--help" ) || options.exists( "-h" ) ) { usage(); }
        comma::csv::format format( av[1] );
        if( !options.exists( "--fields" ) ) { COMMA_THROW( comma::exception, "please specify --fields" ); }
        std::vector< std::string > v = comma::split( options.value< std::string >( "--fields" ), ',' );
        std::vector< comma::csv::format::element > offsets( v.size() );
        for( unsigned int i = 0; i < v.size(); ++i )
        {
            offsets[i] = format.offset( boost::lexical_cast< std::size_t >( v[i] ) - 1 );
        }        
        std::vector< char > w( format.size() ); // stupid windows
        char* buf = &w[0];
        while( std::cin.good() && !std::cin.eof() )
        {
            if( shutdownFlag ) { std::cerr << "csv-bin-cut: interrupted by signal" << std::endl; return -1; }
            // quick and dirty; if performance is an issue, you could read more than
            // one record every time, but absolutely don't make this read blocking!
            // see comma::csv::binary_input_stream::read() for reference - if you know
            // how to do it better, please tell everyone!
            std::cin.read( buf, format.size() );
            if( std::cin.gcount() == 0 ) { continue; }
            if( std::cin.gcount() < int( format.size() ) ) { COMMA_THROW( comma::exception, "expected " << format.size() << " bytes, got only " << std::cin.gcount() ); }
            for( unsigned int i = 0; i < offsets.size(); ++i )
            {
                std::cout.write( buf + offsets[i].offset, offsets[i].size );
            }
        }
        return 0;
    }
    catch( std::exception& ex ) { std::cerr << "csv-bin-cut: " << ex.what() << std::endl; }
    catch( ... ) { std::cerr << "csv-bin-cut: unknown exception" << std::endl; }
    usage();
}        
        
