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

#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <boost/lexical_cast.hpp>
#include "../../application/command_line_options.h"
#include "../../csv/format.h"
#include "../../string/string.h"

static void usage( bool verbose = false )
{
    std::cerr << std::endl;
    std::cerr << "usage: echo \"3f,2f,d\" | csv-format [options] (expand|collapse|count|repeat)" << std::endl;
    std::cerr << std::endl;
    std::cerr << "perform various operations on format strings" << std::endl;
    std::cerr << std::endl;
    std::cerr << "operations" << std::endl;
    std::cerr << "    collapse: output minimal format, e.g. i,i,f,f,f -> 2i,3f" << std::endl;
    std::cerr << "    count: output number of fields, e.g. 2i,3f -> 5" << std::endl;
    std::cerr << "    guess: take a csv string, output (roughly) guessed format; e.g: echo 20170101T000000,1,2,3 | csv-format guess --format ,,ui" << std::endl;
    std::cerr << "    expand: output fully expand format, e.g. 2i,3f -> i,i,f,f,f" << std::endl;
    std::cerr << "    repeat: replicate the format n times, e.g. 2i,3f --count 2 -> 2i,3f,2i,3f" << std::endl;
    std::cerr << "    size: output format size in bytes, e.g: echo 2i,3f | csv-format size would output 20" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options" << std::endl;
    std::cerr << "    guess:" << std::endl;
    std::cerr << "        --delimiter,-d=<delimiter>; default=,; input fields delimiter" << std::endl;
    std::cerr << "        --format=[<format>]; define format of known fields, e.g: --format=,,ui,,,2d" << std::endl;
    std::cerr << "    repeat:" << std::endl;
    std::cerr << "        --count=n: replicate the format n times" << std::endl;
    std::cerr << std::endl;
    exit( 0 );
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
        comma::command_line_options options( ac, av, usage );
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
        else { std::cerr << "csv-format: expected operation; got: \"" << operation << "\"" << std::endl; return 1; }
        while( std::getline( std::cin, line ) )
        {
            const std::string& stripped = comma::strip( line );
            if( !stripped.empty() && stripped[0] != '#' ) { handle( line ); }
        }
        return 0;
    }
    catch( std::exception& ex ) { std::cerr << "csv-format: " << ex.what() << std::endl; }
    catch( ... ) { std::cerr << "csv-format: unknown exception" << std::endl; }
    return 1;
}
