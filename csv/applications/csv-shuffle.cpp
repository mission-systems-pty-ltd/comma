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

#ifdef WIN32
#include <fcntl.h>
#include <io.h>
#endif

#include <iostream>
#include <vector>
#include <comma/application/command_line_options.h>
#include <comma/csv/options.h>
#include <comma/string/string.h>

static void usage( bool verbose )
{
    std::cerr << "perform operations on csv columns" << std::endl;
    std::cerr << std::endl;
    std::cerr << "usage: cat data.csv | csv-shuffle <options> > shuffled.csv" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options" << std::endl;
    std::cerr << "    --help,-h: help; --help --verbose: more help" << std::endl;
    std::cerr << "    --fields,-f <fields>: input fields" << std::endl;
    std::cerr << "    --output-fields,--output,-o <fields>: output fields" << std::endl;
    std::cerr << "        semantics of outputting trailing fields:" << std::endl;
    std::cerr << "            \"--output-fields=x,y\": do not output trailing fields" << std::endl;
    std::cerr << "            \"--output-fields=x,y...\": output trailing fields" << std::endl;
    std::cerr << "            see example below" << std::endl;
    std::cerr << "    --verbose,-v: more output" << std::endl;
    if( verbose ) { std::cerr << std::endl << comma::csv::options::usage() << std::endl; }
    std::cerr << std::endl;
    std::cerr << "examples" << std::endl;
    std::cerr << "    operations (for now): append, remove, swap" << std::endl;
    std::cerr << "    semantics:" << std::endl;
    std::cerr << "        remove:" << std::endl;
    std::cerr << "            cat xyz.csv | csv-shuffle --fields=x,y,z --output-fields=x,z" << std::endl;
    std::cerr << "        append:" << std::endl;
    std::cerr << "            cat xyz.csv | csv-shuffle --fields=x,y,z --output-fields=x,y,z,x" << std::endl;
    std::cerr << "        swap:" << std::endl;
    std::cerr << "            cat xyz.csv | csv-shuffle --fields=x,y,z --output-fields=y,z,x" << std::endl;
    std::cerr << "        remove x, swap y,z, append z two times:" << std::endl;
    std::cerr << "            cat xyz.csv | csv-shuffle --fields=x,y,z --output-fields=z,y,z,z" << std::endl;
    std::cerr << "        do not output trailing fields: swap x and y, do not output z" << std::endl;
    std::cerr << "            cat xyz.csv | csv-shuffle --fields=x,y --output-fields=y,x" << std::endl;
    std::cerr << "        output trailing fields: swap x and y, output z" << std::endl;
    std::cerr << "            cat xyz.csv | csv-shuffle --fields=x,y --output-fields=y,x,..." << std::endl;
    std::cerr << std::endl;
    exit( 1 );
}

struct field
{
    std::string name;
    unsigned int index;
    unsigned int offset;
    boost::optional< unsigned int > input_index;
    unsigned int input_offset;
    unsigned int size;
    field( const std::string& name, unsigned int index ) : name( name ), index( index ) {}
};

int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av );
        bool verbose = options.exists( "--verbose,-v" );
        if( options.exists( "--help,-h" ) ) { usage( verbose ); }
        comma::csv::options csv( options );
        std::vector< std::string > input_fields = comma::split( csv.fields, ',' );
        std::vector< std::string > output_fields = comma::split( options.value< std::string >( "--output-fields,--output,-o" ), ',' );
        bool output_trailing_fields = output_fields.back() == "...";
        if( output_fields.back() == "..." ) { output_fields.erase( output_fields.end() - 1 ); }
        std::vector< field > fields;
        for( unsigned int i = 0; i < output_fields.size(); ++i )
        {
            if( output_fields[i].empty() ) { continue; }
            fields.push_back( field( output_fields[i], i ) );
        }
        if( fields.empty() ) { std::cerr << "csv-shuffle: please define at least one output field" << std::endl; return 1; }
        for( unsigned int i = 0; i < input_fields.size(); ++i )
        {
            for( unsigned int j = 0; j < fields.size(); ++j )
            {
                if( fields[j].name != input_fields[i] ) { continue; }
                fields[j].input_index = i;
                if( csv.binary() )
                {
                    fields[j].input_offset = csv.format().offset( i ).offset;
                    fields[j].size = csv.format().offset( i ).size;
                }
            }
        }
        for( unsigned int i = 0; i < fields.size(); ++i )
        {
            if( !fields[i].input_index ) { std::cerr << "csv-shuffle: \"" << fields[i].name << "\" not found in input fields " << csv.fields << std::endl; return 1; }
        }
        if( csv.binary() )
        {
            #ifdef WIN32
            _setmode( _fileno( stdin ), _O_BINARY );
            _setmode( _fileno( stdout ), _O_BINARY );
            #endif
            std::vector< char > buf( csv.format().size() );
            std::vector< comma::csv::format::element > elements;
            elements.reserve( csv.format().count() ); // quick and dirty, can be really wasteful on large things like images
            for( unsigned int i = 0; i < elements.capacity(); ++i ) { elements.push_back( csv.format().offset( i ) ); }
            while( std::cin.good() && !std::cin.eof() )
            {
                // todo: quick and dirty; if performance is an issue, you could read more than
                // one record every time see comma::csv::binary_input_stream::read() for reference
                std::cin.read( &buf[0], csv.format().size() );
                if( std::cin.gcount() == 0 ) { continue; }
                if( std::cin.gcount() < int( csv.format().size() ) ) { std::cerr << "csv-shuffle: expected " << csv.format().size() << " bytes, got only " << std::cin.gcount() << std::endl; return 1; }
                unsigned int previous_index = 0;
                for( unsigned int i = 0; i < fields.size(); ++i ) // quick and dirty
                {
                    for( unsigned int k = previous_index; k < fields[i].index && k < elements.size(); ++k )
                    {
                        std::cout.write( &buf[ elements[k].offset ], elements[k].size );
                    }
                    std::cout.write( &buf[ fields[i].input_offset ], fields[i].size );
                    previous_index = fields[i].index + 1;
                }
                //std::cerr << "--> previous_index: " << previous_index << " elements.size(): " << elements.size() << std::endl;
                for( unsigned int k = previous_index; output_trailing_fields && k < elements.size(); ++k )
                {
                    std::cout.write( &buf[ elements[k].offset ], elements[k].size );
                }
                std::cout.flush(); // todo: flushing too often?
            }
        }
        else
        {
            while( std::cin.good() && !std::cin.eof() )
            {
                std::string line;
                std::getline( std::cin, line );
                if( !line.empty() && *line.rbegin() == '\r' ) { line = line.substr( 0, line.length() - 1 ); } // windows... sigh...
                if( line.empty() ) { continue; }
                std::vector< std::string > v = comma::split( line, csv.delimiter );
                std::string delimiter;
                unsigned int previous_index = 0;
                for( unsigned int i = 0; i < fields.size(); ++i ) // quick and dirty
                {
                    for( unsigned int k = previous_index; k < fields[i].index && k < v.size(); ++k )
                    {
                        std::cout << delimiter << v[k];
                        delimiter = csv.delimiter;
                    }
                    previous_index = fields[i].index + 1;
                    std::cout << delimiter;
                    if ( *fields[i].input_index < v.size() ) { std::cout << v[ *fields[i].input_index ]; }
                    delimiter = csv.delimiter;
                }
                for( unsigned int k = previous_index; output_trailing_fields && k < v.size(); ++k )
                {
                    std::cout << delimiter << v[k];
                    delimiter = csv.delimiter;
                }
                std::cout << std::endl;
            }
        }
        return 0;
    }
    catch( std::exception& ex )
    {
        std::cerr << "csv-shuffle: " << ex.what() << std::endl;
    }
    catch( ... )
    {
        std::cerr << "csv-shuffle: unknown exception" << std::endl;
    }
    return 1;
}
