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

/// @author vsevolod vlaskine

#include <iostream>
#include <string>
#include <set>
#include <comma/application/contact_info.h>
#include <comma/application/command_line_options.h>
#include <comma/string/string.h>

using namespace comma;

static void usage()
{
    std::cerr << std::endl;
    std::cerr << "various field operations" << std::endl;
    std::cerr << std::endl;
    std::cerr << "usage echo \"hello,,,,world\" | csv-fields [<operation>] [<options>]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options and operations" << std::endl;
    std::cerr << "    numbers (default): convert comma-separated field names to field numbers" << std::endl;
    std::cerr << "                       e.g. for combining with cut or csv-bin-cut" << std::endl;
    std::cerr << std::endl;
    std::cerr << "        --from=<value>: start field numbering from <value>; default=1" << std::endl;
    std::cerr << "                        to keep it consistent with linux cut utility" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    clear: clear some of the field values" << std::endl;
    std::cerr << "        --keep=<fields>: keep given fields, e.g: echo a,b,c | csv-fields clear --keep=a,c outputs a,,c" << std::endl;
    std::cerr << "        --mask=<fields>: keep given fields by position, e.g: echo a,b,c | csv-fields clear --keep=x,,y outputs a,,c" << std::endl;
    std::cerr << std::endl;
    std::cerr << "examples" << std::endl;
    std::cerr << "    echo \"hello,,,,world\" | csv-fields" << std::endl;
    std::cerr << "    1,5" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    echo 1,2,3 | cut -d, -f$( echo ,,world | csv-fields )" << std::endl;
    std::cerr << "    1,5" << std::endl;
    std::cerr << std::endl;
    std::cerr << comma::contact_info << std::endl;
    std::cerr << std::endl;
    exit( 1 );
}

int main( int ac, char** av )
{
    try
    {
        command_line_options options( ac, av );
        if( options.exists( "--help,-h" ) ) { usage(); }
        std::string operation = "numbers";
        const std::vector< std::string > unnamed = options.unnamed( "--help,-h", "-.*" );
        if( !unnamed.empty() ) { operation = unnamed[0]; }
        std::string line;
        std::getline( std::cin, line );
        if( line.empty() ) { std::cerr << "csv-fields: expected fields on stdin, got nothing" << std::endl; return 1; }
        if( operation == "numbers" )
        {
            int from = options.value( "--from", 1 );
            const std::vector< std::string >& v = comma::split( line, ',' );
            std::string comma;
            for( unsigned int i = 0; i < v.size(); ++i )
            {
                if( v[i].empty() ) { continue; }
                std::cout << comma << ( i + from );
                comma = ",";
            }
        }
        else if( operation == "clear" )
        {
            options.assert_mutually_exclusive( "--keep,--mask" );
            std::string keep = options.value< std::string >( "--keep", "" );
            std::string mask = options.value< std::string >( "--mask", "" );
            if( !keep.empty() )
            {
                const std::vector< std::string >& k = comma::split( keep, ',' );
                std::set< std::string > keys;
                for( unsigned int i = 0; i < k.size(); ++i ) { if( !k[i].empty() ) { keys.insert( k[i] ); } }
                const std::vector< std::string >& v = comma::split( line, ',' );
                std::string comma;
                for( unsigned int i = 0; i < v.size(); ++i )
                {
                    std::cout << comma;
                    if( !v[i].empty() && keys.find( v[i] ) != keys.end() ) { std::cout << v[i]; }
                    comma = ",";
                }
            }
            else if( !mask.empty() )
            {
                const std::vector< std::string >& k = comma::split( mask, ',' );
                const std::vector< std::string >& v = comma::split( line, ',' );
                std::string comma;
                for( unsigned int i = 0; i < v.size() && i < k.size(); ++i )
                {
                    std::cout << comma;
                    if( !k[i].empty() ) { std::cout << v[i]; }
                    comma = ",";
                }
            }
            else
            {
                std::cerr << "csv-fields: for clear, please specify --keep or --mask" << std::endl;
                return 1;
            }
        }
        else
        {
            std::cerr << "csv-fields: expected operation, got: \"" << operation << "\"" << std::endl;
            return 1;
        }
        std::cout << std::endl;
        return 0;
    }
    catch( std::exception& ex ) { std::cerr << "csv-fields: " << ex.what() << std::endl; }
    catch( ... ) { std::cerr << "csv-fields: unknown exception" << std::endl; }
    return 1;
}

