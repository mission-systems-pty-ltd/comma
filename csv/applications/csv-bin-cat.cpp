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


/// @author dmitry mikhin

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

static void usage( bool verbose )
{
    std::cerr << std::endl;
    std::cerr << "Usage: csv-bin-cat blah.bin [file2.bin ...] --binary=<format> --fields=<fields> --output-fields=<output-fields>" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    output the specified fields from the binary file(s)" << std::endl;
    std::cerr << "    produce the same output as the more universal csv-shuffle call" << std::endl;
    std::cerr << "        cat blah.bin | csv-shuffle --binary=<format> --fields=<fields> --output-fields=<output-fields>" << std::endl;
    std::cerr << "    but more efficient (MUCH more efficient for large data)" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options" << std::endl;
    std::cerr << "    --help,-h: help; --help --verbose: more help" << std::endl;
    std::cerr << "    --fields,-f,--input-fields <fields>: input fields" << std::endl;
    std::cerr << "    --output-fields,--output,-o <fields>: output fields" << std::endl;
    std::cerr << "    --skip=<N>; skip the first N records (applied once, if multiple files are given; no skip if N = 0)" << std::endl;
    std::cerr << "    --count=<N>; output no more than N records; no output if N = 0" << std::endl;
    std::cerr << "    --verbose,-v: more output" << std::endl;
    std::cerr << std::endl;
    std::cerr << "examples" << std::endl;
    std::cerr << "    csv-bin-cat input.bin --binary=t,s[100000] --fields=t,s --output-fields=t" << std::endl;
    std::cerr << "        output only the time field from a binary file; much more efficient form of" << std::endl;
    std::cerr << "            cat input.bin | csv-shuffle --binary=t,s[100000] --fields=t,s --output-fields=t" << std::endl;
    std::cerr << "    csv-bin-cat input.bin --skip=1000 --binary=t,s[100000] --fields=t,s --output-fields=t | csv-from-bin t" << std::endl;
    std::cerr << "        output only the time field starting with 1000th record; much more efficient form of" << std::endl;
    std::cerr << "            cat input.bin | csv-shuffle --binary=t,s[100000] --fields=t,s --output-fields=t | csv-from-bin t | tail -n+1001" << std::endl;
    std::cerr << "    csv-bin-cat 1.bin 2.bin 3.bin --skip=1000 --head=2 --binary=t,s[100000] --fields=t,s ..." << std::endl;
    std::cerr << "        assuming each of the binary files holds 500 records, output data from" << std::endl;
    std::cerr << "        records 0 and 1 of file '3.bin'; records 0-499 in the file '1.bin' skipped, records 0-499 in" << std::endl;
    std::cerr << "        the file '2.bin' skipped (total of 1000 skipped records), then 2 records output" << std::endl;
    std::cerr << std::endl;
    if ( verbose ) {
        std::cerr << csv::format::usage() << std::endl;
        std::cerr << std::endl;
    }
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
        bool verbose = options.exists( "--verbose,-v" );
        if( ac < 2 || options.exists( "--help" ) || options.exists( "-h" ) ) { usage( verbose ); }
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

