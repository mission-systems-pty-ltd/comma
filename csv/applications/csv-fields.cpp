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
#include <map>
#include "../../application/contact_info.h"
#include "../../application/command_line_options.h"
#include "../../string/string.h"

using namespace comma;

static void usage( bool )
{
    std::cerr << std::endl;
    std::cerr << "various field operations" << std::endl;
    std::cerr << std::endl;
    std::cerr << "usage echo \"hello,,,,world\" | csv-fields [<operation>] [<options>]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options" << std::endl;
    std::cerr << "    --delimiter,-d=<delimiter>; default: , (comma)" << std::endl;
    std::cerr << std::endl;
    std::cerr << "operations" << std::endl;
    std::cerr << "    numbers (default): convert comma-separated field names to field numbers" << std::endl;
    std::cerr << "                       e.g. for combining with cut or csv-bin-cut" << std::endl;
    std::cerr << "        --count,--size: output the total number of fields" << std::endl;
    std::cerr << "        --fill: number even empty fields, e.g. try: echo ,, | csv-fields numbers --fill" << std::endl;
    std::cerr << "        --from=<value>: start field numbering from <value>; default=1" << std::endl;
    std::cerr << "                        to keep it consistent with linux cut utility" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    clear: clear some of the field values" << std::endl;
    std::cerr << "        --keep,--except=<fields>: keep given fields by name" << std::endl;
    std::cerr << "        --mask=<fields>: keep given fields by position" << std::endl;
    std::cerr << "        --remove=<fields>: remove given fields by name, opposite of --keep" << std::endl;
    std::cerr << "        --inverted-mask,--complement-mask,--unmask,--unmasked=<fields>: remove given fields by position, opposite of --mask" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    cut: remove fields" << std::endl;
    std::cerr << "        --fields=<fields>: fields to remove" << std::endl;
    std::cerr << "        --except=<fields>: fields to retain" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    default: set empty fields to default values" << std::endl;
    std::cerr << "        --values=<default values>: e.g: csv-fields default --values=',,,0,0,not-a-date-time'" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    has: check presence of field(s), exit with 1 if fields not present" << std::endl;
    std::cerr << "        --any,--some: check any of the fields are present" << std::endl;
    std::cerr << "        --fields=<fields>: fields to check for" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    prefix: prefix all non-empty field names" << std::endl;
    std::cerr << "        --basename: remove prefix of given fields; incompatible with --path" << std::endl;
    std::cerr << "        --except=<fields>: don't prefix given fields" << std::endl;
    std::cerr << "        --fields=<fields>: prefix only given fields" << std::endl;
    std::cerr << "        --path=<prefix>: path to add" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    rename: rename given fields" << std::endl;
    std::cerr << "        --fields=<fields>: fields to rename" << std::endl;
    std::cerr << "        --to=<fields>: list of new field names" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    strip: remove some leading sub-paths from all or sub-set of fields" << std::endl;
    std::cerr << "        --except=<fields>: don't modify given fields" << std::endl;
    std::cerr << "        --fields=<fields>: modify only given fields" << std::endl;
    std::cerr << "        --path=<prefix>: path to strip" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    make-fixed: normalise input to a fixed number of fields" << std::endl;
    std::cerr << "        --count,--size=<n>: number of output fields" << std::endl;
    std::cerr << "        --force: chop input to <n> fields if larger" << std::endl;
    std::cerr << std::endl;
    std::cerr << "examples" << std::endl;
    std::cerr << "    numbers" << std::endl;
    std::cerr << "        echo \"hello,,,,world\" | csv-fields" << std::endl;
    std::cerr << "        1,5" << std::endl;
    std::cerr << "        echo \"hello,,,world,\" | csv-fields numbers --count" << std::endl;
    std::cerr << "        5" << std::endl;
    std::cerr << std::endl;
    std::cerr << "        echo 1,2,3 | cut -d, -f$( echo ,,world | csv-fields )" << std::endl;
    std::cerr << "        3" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    clear" << std::endl;
    std::cerr << "        mask/unmask first and third field:" << std::endl;
    std::cerr << "        echo a,b,c,d | csv-fields clear --mask X,,Y" << std::endl;
    std::cerr << "        a,,c," << std::endl;
    std::cerr << "        echo a,b,c,d | csv-fields clear --unmask X,,Y" << std::endl;
    std::cerr << "        ,b,,d" << std::endl;
    std::cerr << std::endl;
    std::cerr << "        clear fields by name:" << std::endl;
    std::cerr << "        echo a,b,c,d | csv-fields clear --except a,b" << std::endl;
    std::cerr << "        a,b,," << std::endl;
    std::cerr << "        echo a,b,c,d | csv-fields clear --remove a,b" << std::endl;
    std::cerr << "        ,,c,d" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    prefix" << std::endl;
    std::cerr << "        prefix non-empty fields:" << std::endl;
    std::cerr << "        echo a,,,d | csv-fields prefix --path \"hello/world\"" << std::endl;
    std::cerr << "        hello/world/a,,,hello/world/d" << std::endl;
    std::cerr << std::endl;
    std::cerr << "        leave only basename of paths:" << std::endl;
    std::cerr << "        echo foo/bar/a,,,baz/blah/d | csv-fields prefix --basename" << std::endl;
    std::cerr << "        a,,,d" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    rename" << std::endl;
    std::cerr << "        echo a,b,c,d | csv-fields rename --fields a,c --to x,z" << std::endl;
    std::cerr << "        x,b,z,d" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    strip" << std::endl;
    std::cerr << "        selectively remove part of paths:" << std::endl;
    std::cerr << "        echo foo/bar/a,foo/b,,baz/blah/foo/d | csv-fields strip --path foo" << std::endl;
    std::cerr << "        bar/a,b,,baz/blah/foo/d" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    cut" << std::endl;
    std::cerr << "        cut fields:" << std::endl;
    std::cerr << "        echo a,b,c,d | csv-fields cut --fields b,c" << std::endl;
    std::cerr << "        a,d" << std::endl;
    std::cerr << "        echo a,b,c,d | csv-fields cut --except b,c" << std::endl;
    std::cerr << "        b,c" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    has" << std::endl;
    std::cerr << "        echo a,b,c,d | csv-fields has --fields b,d && echo 'fields present'" << std::endl;
    std::cerr << "        fields present" << std::endl;
    std::cerr << "        echo a,b,c,d | csv-fields has --fields t,b,d || echo 'fields missing'" << std::endl;
    std::cerr << "        fields missing" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    make-fixed" << std::endl;
    std::cerr << "        { echo a,b,c,d; echo x,y,z; } | csv-fields make-fixed --count=6" << std::endl;
    std::cerr << "        a,b,c,d,," << std::endl;
    std::cerr << "        x,y,z,,," << std::endl;
    std::cerr << std::endl;
    std::cerr << "        { echo a,b,c,d; echo x,y,z; } | csv-fields make-fixed --count=3 --force" << std::endl;
    std::cerr << "        a,b,c" << std::endl;
    std::cerr << "        x,y,z" << std::endl;
    std::cerr << std::endl;
    std::cerr << comma::contact_info << std::endl;
    std::cerr << std::endl;
    exit( 1 );
}

int main( int ac, char** av )
{
    try
    {
        command_line_options options( ac, av, usage );
        std::string operation = "numbers";
        const std::vector< std::string > unnamed = options.unnamed( "--help,-h", "-.*" );
        char delimiter = options.value( "--delimiter,-d", ',' );
        if( !unnamed.empty() ) { operation = unnamed[0]; }
        if( operation == "numbers" )
        {
            int from = options.value( "--from", 1 );
            bool fill = options.exists( "--fill" );
            while( std::cin.good() )
            {
                std::string line;
                std::getline( std::cin, line );
                if( line.empty() ) { continue; }
                const std::vector< std::string >& v = comma::split( line, delimiter );
                std::string comma;
                if ( options.exists( "--count,--size" ) ) { std::cout << v.size() << std::endl; continue; }
                for( unsigned int i = 0; i < v.size(); ++i )
                {
                    if( v[i].empty() && !fill ) { continue; }
                    std::cout << comma << ( i + from );
                    comma = ',';
                }
                std::cout << std::endl;
            }
            return 0;
        }
        if( operation == "clear" )
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
                if( line.empty() ) { continue; }
                if( !keep.empty() || !remove.empty() )
                {
                    // todo: quick and dirty, refactor, don't do it for each line
                    const std::vector< std::string >& k = comma::split( keep.empty() ? remove : keep, ',' );
                    std::set< std::string > keys;
                    for( unsigned int i = 0; i < k.size(); ++i ) { if( !k[i].empty() ) { keys.insert( k[i] ); } }
                    const std::vector< std::string >& v = comma::split( line, delimiter );
                    std::string comma;
                    for( unsigned int i = 0; i < v.size(); ++i )
                    {
                        std::cout << comma;
                        if( !v[i].empty() && keep.empty() != ( keys.find( v[i] ) != keys.end() ) ) { std::cout << v[i]; }
                        comma = delimiter;
                    }
                }
                else if( !mask.empty() || !unmasked.empty() )
                {
                    // todo: quick and dirty, refactor, don't do it for each line
                    const std::vector< std::string >& k = comma::split( mask.empty() ? unmasked : mask, ',' );
                    const std::vector< std::string >& v = comma::split( line, delimiter );
                    std::string comma;
                    for( unsigned int i = 0; i < v.size() && i < k.size(); ++i )
                    {
                        std::cout << comma;
                        if( mask.empty() == k[i].empty() ) { std::cout << v[i]; }
                        comma = delimiter;
                    }
                    for( unsigned int i = k.size(); i < v.size(); ++i )
                    { 
                        std::cout << comma;
                        if( mask.empty() ) { std::cout << v[i]; }
                        comma = delimiter;
                    }
                }
                else
                {
                    std::cout << std::string( comma::split( line, delimiter ).size() - 1, delimiter );
                }
                std::cout << std::endl;
            }
            return 0;
        }
        if( operation == "default" )
        {
            const std::vector< std::string >& defaults = comma::split( options.value< std::string >( "--values" ), ',' ); // todo: use specified delimiter instead?
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
        if( operation == "prefix" )
        {
            options.assert_mutually_exclusive( "--fields,--except" );
            const std::string& e = options.value< std::string >( "--except", "" );
            const std::string& f = options.value< std::string >( "--fields", "" );
            const std::string& path = options.value< std::string >( "--path", "" ) + '/';
            bool basename = options.exists( "--basename" );
            if( !basename && path == "/" ) { std::cerr << "csv-fields: please specify --path or --basename" << std::endl; return 1; }
            bool except = !e.empty();
            std::vector< std::string > g;
            if( !e.empty() ) { g = comma::split( e, ',' ); }
            else if( !f.empty() ) { g = comma::split( f, ',' ); }
            std::set< std::string > fields;
            for( unsigned int i = 0; i < g.size(); ++i ) { fields.insert( g[i] ); }
            while( std::cin.good() )
            {
                std::string line;
                std::string comma;
                std::getline( std::cin, line );
                if( line.empty() ) { break; }
                const std::vector< std::string >& v = comma::split( line, delimiter );
                for( unsigned int i = 0; i < v.size(); ++i )
                {
                    std::cout << comma;
                    comma = delimiter;
                    if( ( v[i].empty() || ( !fields.empty() && except == ( fields.find( v[i] ) != fields.end() ) ) ) ) { std::cout << v[i]; }
                    else if( basename ) { std::string::size_type p = v[i].find_last_of( '/' ); std::cout << ( p == std::string::npos ? v[i] : v[i].substr( p + 1 ) ); }
                    else { std::cout << path << v[i]; }
                }
                std::cout << std::endl;
            }
            return 0;
        }
        if( operation == "rename" )
        {
            const std::vector< std::string >& fields = comma::split( options.value< std::string >( "--fields" ), ',' );
            const std::vector< std::string >& to = comma::split( options.value< std::string >( "--to" ), ',' );
            if( fields.size() != to.size() ) { std::cerr << "csv-fields: expected equal number of fields in --fields and --to, got: " << fields.size() << " and " << to.size() << std::endl; return 1; }
            typedef std::map< std::string, std::string > map_t;
            map_t names; // very small map, thus performance should be close to constant time
            for( unsigned int i = 0; i < fields.size(); names[ fields[i] ] = to[i], ++i );
            while( std::cin.good() )
            {
                std::string line;
                std::string comma;
                std::getline( std::cin, line );
                if( line.empty() ) { continue; }
                const std::vector< std::string >& f = comma::split( line, delimiter );
                for( unsigned int i = 0; i < f.size(); ++i )
                {
                    map_t::const_iterator it = names.find( f[i] );
                    std::cout << comma << ( it == names.end() ? f[i] : it->second );
                    comma = delimiter;
                }
                std::cout << std::endl;
            }
            return 0;
        }
        if( operation == "strip" )
        {
            options.assert_mutually_exclusive( "--fields,--except" );
            const std::string& e = options.value< std::string >( "--except", "" );
            const std::string& f = options.value< std::string >( "--fields", "" );
            const std::string& path = options.value< std::string >( "--path" ) + '/';
            bool except = !e.empty();
            std::vector< std::string > g;
            if( !e.empty() ) { g = comma::split( e, ',' ); }
            else if( !f.empty() ) { g = comma::split( f, ',' ); }
            std::set< std::string > fields;
            for( unsigned int i = 0; i < g.size(); ++i ) { fields.insert( g[i] ); }
            while( std::cin.good() )
            {
                std::string line;
                std::string comma;
                std::getline( std::cin, line );
                if( line.empty() ) { continue; }
                const std::vector< std::string >& v = comma::split( line, delimiter );
                for( unsigned int i = 0; i < v.size(); ++i )
                {
                    std::cout << comma;
                    comma = delimiter;
                    if( ( v[i].empty() || ( !fields.empty() && except == ( fields.find( v[i] ) != fields.end() ) ) ) ) { std::cout << v[i]; }
                    else { std::cout << ( v[i].substr( 0, path.length() ) == path ? v[i].substr( path.length() ) : v[i] ); }
                }
                std::cout << std::endl;
            }
            return 0;
        }
        if( operation == "cut" )
        {
            bool except = options.exists( "--except" );
            const std::string& f = except ? options.value< std::string >( "--except", "" ) : options.value< std::string >( "--fields", "" );
            const std::vector< std::string >& s = comma::split( f, delimiter );
            while( std::cin.good() )
            {
                std::string line;
                std::string comma;
                std::getline( std::cin, line );
                if( line.empty() ) { continue; }
                const std::vector< std::string >& v = comma::split( line, delimiter );
                for( unsigned int i = 0; i < v.size(); ++i ) { if( except == ( !v[i].empty() && std::find( s.begin(), s.end(), v[i] ) != s.end() ) ) { std::cout << comma << v[i]; comma = delimiter; } }
                std::cout << std::endl;
            }
            return 0;
        }
        if( operation == "has" )
        {
            const std::string& f = options.value< std::string >( "--fields" );
            const std::vector< std::string >& v = comma::split( f, delimiter );
            const std::set< std::string > fields( v.begin(), v.end() );
            const bool any = options.exists( "--any,--some" );
            std::string line;
            std::getline( std::cin, line );
            if( line.empty() ) { return 1; }
            const std::vector< std::string >& l = comma::split( line, delimiter );
            const std::set< std::string > input( l.begin(), l.end() );
            unsigned int matches = 0;
            for( const auto& field : fields ) { if( input.count( field ) ) { ++matches; if( any ) { break; } } }
            if( !matches ) { return 1; }
            if( any ) { return 0; }
            return matches == fields.size() ? 0 : 1;
        }
        if( operation == "make-fixed" )
        {
            const unsigned int count = options.value< unsigned int >( "--count,--size" );
            bool force = options.exists( "--force" );
            while( std::cin.good() )
            {
                std::string line;
                std::getline( std::cin, line );
                if( line.empty() ) { continue; }
                const std::vector< std::string >& v = comma::split( line, delimiter );
                if( v.size() <= count )
                {
                    std::cout << line;
                    for( unsigned int i = v.size(); i < count; i++ ) { std::cout << delimiter; }
                }
                else
                {
                    if( !force ) { std::cerr << "csv-fields: make-fixed of " << count << " fields but found " << v.size() << " fields in line: \"" << line << "\". Use --force to crop line to " << count << " fields." << std::endl; return 1; }
                    std::string d;
                    for( unsigned int i = 0; i < count; d = delimiter, ++i ) { std::cout << d << v[i]; }
                }
                std::cout << std::endl;
            }
            return 0;
        }
        std::cerr << "csv-fields: expected operation, got: \"" << operation << "\"" << std::endl;
        return 1;
    }
    catch( std::exception& ex ) { std::cerr << "csv-fields: " << ex.what() << std::endl; }
    catch( ... ) { std::cerr << "csv-fields: unknown exception" << std::endl; }
    return 1;
}

