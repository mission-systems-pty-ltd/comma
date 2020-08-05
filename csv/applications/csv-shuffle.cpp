// Copyright (c) 2011 The University of Sydney
// Copyright (c) 2020 Vsevolod Vlaskine

#ifdef WIN32
#include <fcntl.h>
#include <io.h>
#endif

#include <iostream>
#include <vector>
#include "../../application/command_line_options.h"
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

struct field
{
    std::string name;
    unsigned int index;
    boost::optional< unsigned int > input_index;
    unsigned int input_offset;
    unsigned int size;
    field( const std::string& name, unsigned int index ) : name( name ), index( index ), input_offset( 0 ), size( 0 ) {}
};

int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av, usage );
        comma::csv::options csv( options );
        csv.fields = options.value< std::string >( "--input-fields,--fields,-f", "" );
        std::vector< std::string > input_fields = comma::split( csv.fields, ',', true );
        std::vector< std::string > output_fields = comma::split( options.value< std::string >( "--output-fields,--output,-o", csv.fields ), ',', true );
        if( output_fields.back() == "..." ) { std::cerr << "csv-shuffle: support for trailing fields has been removed for now; please specify input/output fields explicitly" << std::endl; return 1; }
        std::vector< field > fields;
        if( csv.binary() )
        {
            for( unsigned int i = 0; i < output_fields.size(); )
            {
                fields.push_back( field( output_fields[i], i ) );
                unsigned int j = 0;
                for( ; j < input_fields.size() && input_fields[j] != output_fields[i]; ++j );
                if( j >= input_fields.size() ) { std::cerr << "csv-shuffle: output field '" << output_fields[i] << "' not found in input fields '" << csv.fields << "'" << std::endl; return 1; }
                fields.back().input_offset = csv.format().offset( j ).offset;
                for( ; i < output_fields.size() && j < input_fields.size() && input_fields[j] == output_fields[i]; ++i, ++j ) { fields.back().size += csv.format().offset( j ).size; }
            }
            //for( unsigned int i = 0; i < fields.size(); ++i ) { std::cerr << "--> i: " << i << " fields[i].name: " << fields[i].name << " fields[i].input_offset: " << fields[i].input_offset << " fields[i].size: " << fields[i].size << std::endl; }
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
                for( unsigned int i = 0; i < fields.size(); ++i ) { std::cout.write( &buf[ fields[i].input_offset ], fields[i].size ); }
                if( csv.flush ) { std::cout.flush(); }
            }
        }
        else
        {
            for( unsigned int i = 0; i < output_fields.size(); ++i ) { fields.push_back( field( output_fields[i], i ) ); }
            for( unsigned int i = 0; i < input_fields.size(); ++i )
            {
                for( unsigned int j = 0; j < fields.size(); ++j )
                {
                    if( fields[j].name == input_fields[i] ) { fields[j].input_index = i; }
                }
            }
            for( unsigned int i = 0; i < fields.size(); ++i )
            {
                if( !fields[i].input_index ) { std::cerr << "csv-shuffle: \"" << fields[i].name << "\" not found in input fields " << csv.fields << std::endl; return 1; }
            }
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
                std::cout << std::endl;
            }
        }
        return 0;
    }
    catch( std::exception& ex ) { std::cerr << "csv-shuffle: " << ex.what() << std::endl; }
    catch( ... ) { std::cerr << "csv-shuffle: unknown exception" << std::endl; }
    return 1;
}
