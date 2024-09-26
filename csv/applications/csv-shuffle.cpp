// Copyright (c) 2011 The University of Sydney
// Copyright (c) 2020 Vsevolod Vlaskine

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
    std::cerr << R"(
swap, remove, or duplicate csv fields

usage: cat data.csv | csv-shuffle <options> > shuffled.csv

options
    --drop-empty,-e; e.g. two following commands are equivalent
                     csv-shuffle --fields a,b,,,c --drop-empty
                     csv-shuffle --fields a,b,,,c --output-fields a,b,c
    --fields,-f,--input-fields=<fields>; input fields
    --output-fields,--output,-o=<fields>; output fields, if not specified,
                                          will be set to --input-fields,
                                          which would chop off trailing
                                          input fields see also --drop-empty
    --verbose,-v: more verbose output
)" << std::endl;
    std::cerr << "csv options" << std::endl;
    std::cerr << comma::csv::options::usage( verbose ) << std::endl;
std::cerr << R"(examples
    remove
        echo 0,1,2 | csv-shuffle --fields=x,y,z
    append
        echo 0,1,2 | csv-shuffle --fields=x,y,z --output-fields=x,y,z,x
    swap
        echo 0,1,2 | csv-shuffle --fields=x,y,z --output-fields=y,z,x
    remove x, swap y,z, append z two times
        echo 0,1,2 | csv-shuffle --fields=x,y,z --output-fields=z,y,z,z
)" << std::endl;
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
        if( options.exists( "--drop-empty,-e" ) )
        {
            std::vector< std::string > v;
            for( auto s: output_fields ) { if( !s.empty() ) { v.push_back( s ); } }
            output_fields = v;
        }
        COMMA_ASSERT_BRIEF( !output_fields.empty(), "please specify --output-fields or --drop-empty" );
        COMMA_ASSERT_BRIEF( output_fields.back() != "...", "support for trailing fields has been removed for now; please specify input/output fields explicitly" );
        auto find_ = [&]( const std::string& n )->unsigned int
        {
            COMMA_ASSERT_BRIEF( !n.empty(), "got empty fields in output fields '" << comma::join( output_fields, ',' ) << "'; you may need to use --drop-empty" );
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
                COMMA_ASSERT_BRIEF( std::cin.gcount() >= int( csv.format().size() ), "expected " << csv.format().size() << " bytes, got only " << std::cin.gcount() );
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
            COMMA_ASSERT_BRIEF( v.size() >= input_fields.size(), "expected at least " << input_fields.size() << " fields, got only " << v.size() << " in record \"" << line << "\"" );
            std::string delimiter;
            for( auto index: indices ) { std::cout << delimiter << v[index]; delimiter = csv.delimiter; }
            std::cout << std::endl;
        }
        return 0;
    }
    catch( std::exception& ex ) { comma::say() << ex.what() << std::endl; }
    catch( ... ) { comma::say() << "unknown exception" << std::endl; }
    return 1;
}
