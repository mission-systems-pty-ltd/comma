// Copyright (c) 2011 The University of Sydney

/// @author vsevolod vlaskine

#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <boost/lexical_cast.hpp>
#include "../../application/command_line_options.h"
#include "../../csv/format.h"
#include "../../string/string.h"

using namespace comma;
static const char *app_name = "csv-format";

static void usage( bool verbose = false )
{
    std::cerr << std::endl;
    std::cerr << "usage: echo \"3f,2f,d\" | " << app_name << " [options] (expand|collapse|count|repeat)" << std::endl;
    std::cerr << std::endl;
    std::cerr << "perform various operations on format strings" << std::endl;
    std::cerr << std::endl;
    std::cerr << "operations" << std::endl;
    std::cerr << "    collapse: output minimal format, e.g. i,i,f,f,f -> 2i,3f" << std::endl;
    std::cerr << "    count: output number of fields, e.g. 2i,3f -> 5" << std::endl;
    std::cerr << "    guess: take a csv string, output (roughly) guessed format; e.g: echo 20170101T000000,1,2,3 | csv-format guess --format ,,ui" << std::endl;
    std::cerr << "    expand: output fully expand format, e.g. 2i,3f -> i,i,f,f,f" << std::endl;
    std::cerr << "    repeat: replicate the format n times, e.g. 2i,3f --count 2 -> 2i,3f,2i,3f" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options" << std::endl;
    std::cerr << "    guess:" << std::endl;
    std::cerr << "        --delimiter,-d=<delimiter>; default=,; input fields delimiter" << std::endl;
    std::cerr << "        --format=[<format>]; define format of known fields, e.g: --format=,,ui,,,2d" << std::endl;
    std::cerr << "    repeat:" << std::endl;
    std::cerr << "        --count=n: replicate the format n times" << std::endl;
    std::cerr << std::endl;
    exit( 1 );
}

static std::string incomplete_expanded( const std::string& s ) // quick and dirty
{
    const std::vector< std::string > v = comma::split( s, ',' );
    std::ostringstream oss;
    std::string comma;
    for( const auto& i: v )
    {
        oss << comma;
        if( !i.empty() ) { oss << comma::csv::format( i ).expanded_string(); }
        comma = ",";
    }
    return oss.str();
}

static std::string guessed_format_of( const std::string& s ) // super-quick and dirty
{
    try
    {
        boost::lexical_cast< int >( s );
        return "i";
    }
    catch( ... )
    { 
        try
        { 
            boost::lexical_cast< double >( s );
            return "d";
        }
        catch( ... )
        {
            try
            {
                boost::posix_time::from_iso_string( s );
                return "t";
            }
            catch( ... )
            {
                return "s[" + boost::lexical_cast< std::string >( s.size() ) + "]";
            }
        }
    }
}

int main( int ac, char** av )
{
    try
    {
        command_line_options options( ac, av, usage );
        const std::vector< std::string > unnamed = options.unnamed( "--help,-h", "-.*" );
        if( unnamed.empty() ) { std::cerr << "csv-format: please specify operation" << std::endl; return 1; }
        if( unnamed.size() > 1 ) { std::cerr << "csv-format: please only one operation" << std::endl; return 1; }
        std::string operation = unnamed[0];
        std::string line;
        std::function< void( const std::string& ) > handle;
        if( operation == "expand" ) { handle = [&]( const std::string& s ) { std::cout << comma::csv::format( s ).expanded_string() << std::endl; }; }
        else if( operation == "collapse" ) { handle = [&]( const std::string& s ) { std::cout << comma::csv::format( s ).collapsed_string() << std::endl; }; }
        else if( operation == "count" ) { handle = [&]( const std::string& s ) { std::cout << comma::csv::format( s ).count() << std::endl; }; }
        else if( operation == "size" ) { handle = [&]( const std::string& s ) { std::cout << comma::csv::format( s ).size() << std::endl; }; }
        else if( operation == "guess" ) { handle = [&]( const std::string& s ) {
                                                                                    static const std::vector< std::string >& e = comma::split( incomplete_expanded( options.value< std::string >( "--format", "" ) ), ',' );
                                                                                    static char delimiter = options.value( "--delimiter,-d", ',' );
                                                                                    const std::vector< std::string >& v = comma::split( s, delimiter );
                                                                                    std::string comma;
                                                                                    for( unsigned int i = 0; i < v.size(); ++i ) // quick and dirty
                                                                                    {
                                                                                        std::cout << comma << ( i < e.size() && !e[i].empty() ? e[i] : guessed_format_of( v[i] ) );
                                                                                        comma = ",";
                                                                                    }
                                                                                    std::cout << std::endl;
                                                                                }; }
        else if( operation == "repeat" ) { handle = [&]( const std::string& s ) {
                                                                                    static unsigned int count = options.value< unsigned int >( "--count" );
                                                                                    comma::csv::format format( s );
                                                                                    if( count )
                                                                                    {
                                                                                        std::cout << format.string();
                                                                                        for ( unsigned int i = 0; i < count - 1; ++i ) { std::cout << "," << format.string(); }
                                                                                        std::cout << std::endl;
                                                                                    }
                                                                                }; }
        else { std::cerr << app_name << ": expected operation; got: \"" << operation << "\"" << std::endl; return 1; }
        while( std::getline( std::cin, line ) )
        {
            const std::string& stripped = comma::strip( line );
            if( !stripped.empty() && stripped[0] != '#' ) { handle( line ); }
        }
        return 0;
    }
    catch( std::exception& ex ) { std::cerr << app_name << ": " << ex.what() << std::endl; }
    catch( ... ) { std::cerr << app_name << ": unknown exception" << std::endl; }
    return 1;
}
