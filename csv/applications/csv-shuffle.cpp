// This file is part of comma, a generic and flexible library
// Copyright (c) 2011 The University of Sydney
// Copyright (c) 2020 Vsevolod Vlaskine
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
// Copyright (c) 2011 The University of Sydney

/// @author vsevolod vlaskine

#ifdef WIN32
#include <fcntl.h>
#include <io.h>
#endif

#include <iostream>
#include <vector>
#include "../../application/command_line_options.h"
#include "../../base/exception.h"
#include "../../csv/options.h"
#include "../../string/string.h"

static void usage( bool verbose )
{
    std::cerr << "perform operations on csv columns" << std::endl;
    std::cerr << std::endl;
    std::cerr << "usage: cat data.csv | csv-shuffle <options> > shuffled.csv" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options" << std::endl;
    std::cerr << "    --help,-h: help; --help --verbose: more help" << std::endl;
    std::cerr << "    --fields,-f,--input-fields=<fields>; input fields" << std::endl;
    std::cerr << "    --output-fields,--output,-o=<fields>; output fields, if not specified, will be set" << std::endl;
    std::cerr << "                                          to --input-fields, which would chops off trailing input fields" << std::endl;
    std::cerr << "    --verbose,-v: more output" << std::endl;
    if( verbose ) { std::cerr << std::endl << comma::csv::options::usage() << std::endl; }
    std::cerr << std::endl;
    std::cerr << "examples" << std::endl;
    std::cerr << "    operations (for now): append, remove, swap" << std::endl;
    std::cerr << "    semantics:" << std::endl;
    std::cerr << "        remove:" << std::endl;
    std::cerr << "            echo 0,1,2 | csv-shuffle --fields=x,y,z" << std::endl;
    std::cerr << "        append:" << std::endl;
    std::cerr << "            echo 0,1,2 | csv-shuffle --fields=x,y,z --output-fields=x,y,z,x" << std::endl;
    std::cerr << "        swap:" << std::endl;
    std::cerr << "            echo 0,1,2 | csv-shuffle --fields=x,y,z --output-fields=y,z,x" << std::endl;
    std::cerr << "        remove x, swap y,z, append z two times:" << std::endl;
    std::cerr << "            echo 0,1,2 | csv-shuffle --fields=x,y,z --output-fields=z,y,z,z" << std::endl;
    std::cerr << std::endl;
    exit( 0 );
}

int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av, usage );
        comma::csv::options csv( options, options.value< std::string >( "--fields,-f,--input-fields" ) );
        std::vector< std::string > input_fields = comma::split( csv.fields, ',', true );
        std::vector< std::string > output_fields = comma::split( options.value< std::string >( "--output-fields,--output,-o", csv.fields ), ',', true );
        if( output_fields.back() == "..." ) { std::cerr << "csv-shuffle: support for trailing fields has been removed for now; please specify input/output fields explicitly" << std::endl; return 1; }
        auto find_ = [&]( const std::string& n )->unsigned int
        {
            if( n.empty() ) { COMMA_THROW( comma::exception, "got empty fields in output fields '" << comma::join( output_fields, ',' ) << "'" ); }
            unsigned int j = 0;
            for( ; j < input_fields.size(); ++j ) { if( input_fields[j] == n ) { return j; } }
            COMMA_THROW( comma::exception, "output field '" << n << "' not found in input fields '" << csv.fields << "'" );
        };
        if( csv.binary() )
        {
            std::vector< std::pair< unsigned int, unsigned int > > offsets;
            for( unsigned int i = 0; i < output_fields.size(); )
            {
                unsigned int j = find_( output_fields[i] );
                offsets.push_back( std::make_pair( csv.format().offset( j ).offset, 0 ) );
                for( ; i < output_fields.size() && j < input_fields.size() && input_fields[j] == output_fields[i]; ++i, ++j ) { offsets.back().second += csv.format().offset( j ).size; }
            }
            #ifdef WIN32
            _setmode( _fileno( stdin ), _O_BINARY );
            _setmode( _fileno( stdout ), _O_BINARY );
            #endif
            std::vector< char > buf( csv.format().size() );
            if( !csv.flush ) { std::cin.tie( NULL ); } // quick and dirty; std::cin is tied to std::cout by default, which is thread-unsafe now
            while( std::cin.good() && !std::cin.eof() )
            {
                std::cin.read( &buf[0], csv.format().size() );
                if( std::cin.gcount() == 0 ) { continue; }
                if( std::cin.gcount() < int( csv.format().size() ) ) { std::cerr << "csv-shuffle: expected " << csv.format().size() << " bytes, got only " << std::cin.gcount() << std::endl; return 1; }
                for( const auto& offset: offsets ) { std::cout.write( &buf[ offset.first ], offset.second ); }
                if( csv.flush ) { std::cout.flush(); }
            }
            return 0;
        }
        std::vector< unsigned int > indices;
        for( const auto& field: output_fields ) { indices.push_back( find_( field ) ); }
        while( std::cin.good() && !std::cin.eof() )
        {
            std::string line;
            std::getline( std::cin, line );
            if( !line.empty() && *line.rbegin() == '\r' ) { line = line.substr( 0, line.length() - 1 ); } // windows... sigh...
            if( line.empty() ) { continue; }
            const auto& v = comma::split( line, csv.delimiter );
            if( v.size() < input_fields.size() ) { std::cerr << "csv-shuffle: expected at least " << input_fields.size() << " fields, got only " << v.size() << " in record \"" << line << "\"" << std::endl; return 1; }
            std::string delimiter;
            for( auto index: indices ) { std::cout << delimiter << v[index]; delimiter = csv.delimiter; }
            std::cout << std::endl;
        }
        return 0;
    }
    catch( std::exception& ex ) { std::cerr << "csv-shuffle: " << ex.what() << std::endl; }
    catch( ... ) { std::cerr << "csv-shuffle: unknown exception" << std::endl; }
    return 1;
}
