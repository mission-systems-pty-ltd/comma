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
    std::cerr << "        --keep,--except=<fields>: keep given fields by name" << std::endl;
    std::cerr << "        --mask=<fields>: keep given fields by position" << std::endl;
    std::cerr << "        --remove=<fields>: remove given fields by name, opposite of --keep" << std::endl;
    std::cerr << "        --inverted-mask,--complement-mask,--unmask,--unmasked=<fields>: remove given fields by position, opposite of --mask" << std::endl;
    std::cerr << std::endl;
    std::cerr << "examples" << std::endl;
    std::cerr << "    numbers" << std::endl;
    std::cerr << "        echo \"hello,,,,world\" | csv-fields" << std::endl;
    std::cerr << "        1,5" << std::endl;
    std::cerr << std::endl;
    std::cerr << "        echo 1,2,3 | cut -d, -f$( echo ,,world | csv-fields )" << std::endl;
    std::cerr << "        3" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    clear" << std::endl;
    std::cerr << "        mask/unmask first and third field:" << std::endl;
    std::cerr << "        echo a,b,c,d | csv-fields clear --mask X,,Y" << std::endl;
    std::cerr << "        echo a,,c" << std::endl;
    std::cerr << "        echo a,b,c,d | csv-fields clear --unmask X,,Y" << std::endl;
    std::cerr << "        echo ,b,,d" << std::endl;
    std::cerr << std::endl;
    std::cerr << "        clear fields by name:" << std::endl;
    std::cerr << "        echo a,b,c,d | csv-fields clear --except a,b" << std::endl;
    std::cerr << "        echo a,b,," << std::endl;
    std::cerr << "        echo a,b,c,d | csv-fields clear --remove a,b" << std::endl;
    std::cerr << "        echo ,,c,d" << std::endl;
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
        if( operation == "numbers" )
        {
            int from = options.value( "--from", 1 );
            while( std::cin.good() )
            {
                std::string line;
                std::getline( std::cin, line );
                if( line.empty() ) { break; }
                const std::vector< std::string >& v = comma::split( line, ',' );
                std::string comma;
                for( unsigned int i = 0; i < v.size(); ++i )
                {
                    if( v[i].empty() ) { continue; }
                    std::cout << comma << ( i + from );
                    comma = ",";
                }
            }
            std::cout << std::endl;
        }
        else if( operation == "clear" )
        {
            options.assert_mutually_exclusive( "--except,--keep,--mask,--remove,--inverted-mask,--complement-mask,--unmask,--unmasked" );
            std::string keep = options.value< std::string >( "--keep,--except", "" );
            std::string remove = options.value< std::string >( "--remove", "" );
            std::string mask = options.value< std::string >( "--mask", "" );
            std::string unmasked = options.value< std::string >( "--inverted-mask,--complement-mask,--unmask,--unmasked", "" );
            while( std::cin.good() )
            {
                std::string line;
                std::getline( std::cin, line );
                if( line.empty() ) { break; }
                if( !keep.empty() || !remove.empty() )
                {
                    // todo: quick and dirty, refactor, don't do it for each line
                    const std::vector< std::string >& k = comma::split( keep.empty() ? remove : keep, ',' );
                    std::set< std::string > keys;
                    for( unsigned int i = 0; i < k.size(); ++i ) { if( !k[i].empty() ) { keys.insert( k[i] ); } }
                    const std::vector< std::string >& v = comma::split( line, ',' );
                    std::string comma;
                    for( unsigned int i = 0; i < v.size(); ++i )
                    {
                        std::cout << comma;
                        if( !v[i].empty() && keep.empty() != ( keys.find( v[i] ) != keys.end() ) ) { std::cout << v[i]; }
                        comma = ",";
                    }
                }
                else if( !mask.empty() || !unmasked.empty() )
                {
                    // todo: quick and dirty, refactor, don't do it for each line
                    const std::vector< std::string >& k = comma::split( mask.empty() ? unmasked : mask, ',' );
                    const std::vector< std::string >& v = comma::split( line, ',' );
                    std::string comma;
                    for( unsigned int i = 0; i < v.size() && i < k.size(); ++i )
                    {
                        std::cout << comma;
                        if( mask.empty() == k[i].empty() ) { std::cout << v[i]; }
                        comma = ",";
                    }
                    for( unsigned int i = k.size(); i < v.size(); ++i )
                    { 
                        std::cout << comma;
                        if( mask.empty() ) { std::cout << v[i]; }
                        comma = ",";
                    }
                }
                else
                {
                    std::cout << std::string( comma::split( line, ',' ).size() - 1, ',' );
                }
                std::cout << std::endl;
            }
        }
        else
        {
            std::cerr << "csv-fields: expected operation, got: \"" << operation << "\"" << std::endl;
            return 1;
        }
        return 0;
    }
    catch( std::exception& ex ) { std::cerr << "csv-fields: " << ex.what() << std::endl; }
    catch( ... ) { std::cerr << "csv-fields: unknown exception" << std::endl; }
    return 1;
}

