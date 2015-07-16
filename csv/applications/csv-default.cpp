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

#include <iostream>
#include <comma/application/command_line_options.h>
#include <comma/application/contact_info.h>
#include <comma/string/string.h>

using namespace comma;

static void usage( bool )
{
    std::cerr << std::endl;
    std::cerr << "fill empty fields with given defaults: e.g:" << std::endl;
    std::cerr << "> echo 1,,3 | csv-default ',0,0'" << std::endl;
    std::cerr << "1,0,3" << std::endl;
    std::cerr << std::endl;
    std::cerr << "usage: cat data.csv | csv-default <default values> [options] > data.empty-fields-filled.csv" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options" << std::endl;
    std::cerr << "    --delimiter=[<delimiter>]; default: , (comma)" << std::endl;
    std::cerr << std::endl;
    std::cerr << comma::contact_info << std::endl;
    std::cerr << std::endl;
    exit( 0 );
}

int main( int ac, char** av )
{
    try
    {
        if( ac < 2 ) { std::cerr << "csv-default: please specify default values" << std::endl; return 1; }
        command_line_options options( ac, av, usage );
        char delimiter = options.value( "--delimiter", ',' );
        const std::vector< std::string >& defaults = comma::split( av[1], ',' ); // todo: use specified delimiter instead?
        std::string line;
        line.reserve( 4000 );
        while( std::cin.good() && !std::cin.eof() )
        {
            std::getline( std::cin, line );
            if( !line.empty() && *line.rbegin() == '\r' ) { line = line.substr( 0, line.length() - 1 ); } // windows... sigh...
            if( line.empty() ) { continue; }
            const std::vector< std::string >& values = comma::split( line, delimiter );
            std::string d;
            for( unsigned int i = 0; i < values.size(); d = delimiter, ++i ) { std::cout << d << ( values[i].empty() && i < defaults.size() ? defaults[i] : values[i] ); }
            std::cout << std::endl;
        }
        return 0;
    }
    catch( std::exception& ex ) { std::cerr << "csv-default: " << ex.what() << std::endl; }
    catch( ... ) { std::cerr << "csv-default: unknown exception" << std::endl; }
    return 1;
}
