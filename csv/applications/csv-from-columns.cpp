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
//    This product includes software developed by the University of Sydney.
// 4. Neither the name of the University of Sydney nor the
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

#include <iostream>
#include <boost/lexical_cast.hpp>
#include <comma/application/command_line_options.h>

static void usage()
{
    std::cerr << std::endl;
    std::cerr << "take fixed-column format, output csv" << std::endl;
    std::cerr << "trailing whitespaces will be removed" << std::endl;
    std::cerr << "(since some systems still use fixed-column files)" << std::endl;
    std::cerr << std::endl;
    std::cerr << "usage: cat fixed-column.txt | csv-from-columns <sizes> [<options>] > fixed-columns.csv" << std::endl;
    std::cerr << std::endl;
    std::cerr << "sizes: field sizes in bytes, e.g. \"8,4,4\"" << std::endl;
    std::cerr << "       shortcuts: \"8,3*4\" is the same as \"8,4,4,4\"" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options" << std::endl;
    std::cerr << "    --delimiter,-d=<delimiter>: output delimiter; default: \",\"" << std::endl;
    std::cerr << std::endl;
    exit( 1 );
}

int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av );
        if( options.exists( "--help,-h" ) ) { usage(); }
        char delimiter = options.value( "--delimiter,-d", ',' );
        std::vector< std::string > unnamed = options.unnamed( "", "--delimiter,-d" );
        if( unnamed.empty() ) { std::cerr << "csv-from-columns: expected column sizes, got none" << std::endl; return 1; }
        std::vector< unsigned int > sizes;
        std::vector< std::string > v = comma::split( unnamed[0], ',' );
        for( std::size_t i = 0; i < v.size(); ++i )
        {
            std::vector< std::string > w = comma::split( v[i], '*' );
            if( w[0].empty() ) { std::cerr << "csv-from-columns: invalid column size format: " << unnamed[0] << std::endl; return 1; }
            switch( w.size() )
            {
                case 1:
                    sizes.push_back( boost::lexical_cast< unsigned int >( w[0] ) );
                    break;
                case 2:
                    sizes.resize( sizes.size() + boost::lexical_cast< unsigned int >( w[0] ), boost::lexical_cast< unsigned int >( w[1] ) );
                    break;
                default:
                    std::cerr << "csv-from-columns: invalid column size format: " << unnamed[0] << std::endl;
                    return 1;
            }
        }
        while( std::cin.good() && !std::cin.eof() )
        {
            std::string line;
            std::getline( std::cin, line );
            if( line.empty() ) { break; }
            std::size_t size = line.size() - 1; // eat end of line
            if( line.empty() ) { continue; }
            const char* it = &line[0];
            const char* end = &line[0] + size;
            std::string d;
            for( std::size_t i = 0; i < sizes.size(); it += sizes[i], ++i )
            {
                std::cout << d;
                if( it < end )
                {
                    std::string s( it, ( it + sizes[i] ) > end ? end - it : sizes[i] );
                    std::string::size_type first = s.find_first_not_of( ' ' );
                    std::string::size_type last = s.find_last_not_of( ' ' );
                    if( last != std::string::npos ) { std::cout.write( it + first, last - first + 1 ); }
                }
                d = delimiter;
            }
            std::cout << std::endl;
        }
        return 0;
    }
    catch( std::exception& ex )
    {
        std::cerr << "csv-from-columns: " << ex.what() << std::endl;
    }
    catch( ... )
    {
        std::cerr << "csv-from-columns: unknown exception" << std::endl;
    }
    return 1;
}
