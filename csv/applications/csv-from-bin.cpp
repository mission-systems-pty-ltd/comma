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


/// @author vsevolod vlaskine

#ifdef WIN32
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#endif

#include <stdlib.h>
#include <iostream>
#include "../../application/command_line_options.h"
#include "../../application/contact_info.h"
#include "../../base/exception.h"
#include "../../csv/format.h"
#include "../../string/string.h"

using namespace comma;

static void usage()
{
    std::cerr << std::endl;
    std::cerr << "Usage: cat blah.bin | csv-from-bin <format> --precision <precision> > blah.csv" << std::endl;
    std::cerr << std::endl;
    std::cerr << "--precision: set precision (number of mantissa digits) for floating point types" << std::endl;
    std::cerr << csv::format::usage() << std::endl;
    std::cerr << std::endl;
    std::cerr << comma::contact_info << std::endl;
    std::cerr << std::endl;
    exit( 0 );
}

int main( int ac, char** av )
{
    #ifdef WIN32
    _setmode( _fileno( stdin ), _O_BINARY ); /// @todo move to a library
    #endif
    try
    {
        command_line_options options( ac, av );
        if( ac < 2 || options.exists( "--help" ) || options.exists( "-h" ) ) { usage(); }
        char delimiter = options.value( "--delimiter", ',' );
        boost::optional< unsigned int > precision;
        if( options.exists( "--precision" ) ) { precision = options.value< unsigned int >( "--precision" ); }
        comma::csv::format format( av[1] );
        std::vector< char > w( format.size() ); //char buf[ format.size() ]; // stupid windows
        char* buf = &w[0];
        while( std::cin.good() && !std::cin.eof() )
        {
            std::cin.read( buf, format.size() );
            if( std::cin.gcount() == 0 ) { break; }
            if( std::cin.gcount() < static_cast< int >( format.size() ) ) { COMMA_THROW( comma::exception, "expected " << format.size() << " bytes, got only " << std::cin.gcount() ); }
            std::cout << format.bin_to_csv( buf, delimiter, precision ) << std::endl;
        }
        return 0;
    }
    catch( std::exception& ex ) { std::cerr << "csv-from-bin: " << ex.what() << std::endl; }
    catch( ... ) { std::cerr << "csv-from-bin: unknown exception" << std::endl; }
    return 1;
}

