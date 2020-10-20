// Copyright (c) 2011 The University of Sydney

/// @author vsevolod vlaskine

#include <functional>
#include <iostream>
#include <string>
#include <set>
#include <map>
#include <unordered_map>
#include "../../application/command_line_options.h"
#include "../../string/string.h"

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
    std::cerr << "        --fields=[<fields>]: number only fields with given names, same as csv-fields clear --except ... | csv-fields numbers" << std::endl;
    std::cerr << "        --fill: number even empty fields, e.g. try: echo ,, | csv-fields numbers --fill" << std::endl;
    std::cerr << "        --from=<value>: start field numbering from <value>; default=1" << std::endl;
    std::cerr << "                        to keep it consistent with linux cut utility" << std::endl;
    std::cerr << "        --prefix: if --fill, field name prefix, e.g. try: echo ,, | csv-fields numbers --fill --prefix f" << std::endl;
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
    std::cerr << "        --empty: remove empty fields, e.g. csv-fields cut --empty <<< 'a,,,b,,c' will output a,b,c" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    default: set empty fields to default values" << std::endl;
    std::cerr << "        --value=<default value>: fill all empty fields with the same value" << std::endl;
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
    std::cerr << "        --values=[<values>]: fill missing fields with given values" << std::endl;
    std::cerr << "                             if --count not specified, use number of <values> as desired number of fields" << std::endl;
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
    std::cerr << "    make-fixed" << std::endl;
    std::cerr << "        { echo a,b; echo x,y,z; } | csv-fields make-fixed --count=6 --fields=A,B,C,D,E,F" << std::endl;
    std::cerr << "        a,b,C,D,E,F" << std::endl;
    std::cerr << "        x,y,z,D,E,F" << std::endl;
    std::cerr << std::endl;
    std::cerr << "        { echo a,b,c,d; echo x,y,z; } | csv-fields make-fixed --count=3 --force" << std::endl;
    std::cerr << "        a,b,c" << std::endl;
    std::cerr << "        x,y,z" << std::endl;
    std::cerr << std::endl;
    exit( 0 );
}

int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av, usage );
        std::string operation = "numbers";
        const std::vector< std::string > unnamed = options.unnamed( "--help,-h", "-.*" );
        char delimiter = options.value( "--delimiter,-d", ',' );
        if( !unnamed.empty() ) { operation = unnamed[0]; }
        auto numbers = [&]()->int
        {
            int from = options.value( "--from", 1 );
            bool fill = options.exists( "--fill" );
            options.assert_mutually_exclusive( "--fill,--fields" );
            const auto& v = comma::split( options.value< std::string >( "--fields", "" ), ',', true );
            std::set< std::string > fields( v.begin(), v.end() );
            std::string prefix = options.value< std::string >( "--prefix", "" );            
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
                    if( !fields.empty() && fields.find( v[i] ) == fields.end() ) { continue; }
                    std::cout << comma << prefix << ( i + from );
                    comma = ',';
                }
                std::cout << std::endl;
            }
            return 0;
        };
        auto clear = [&]()->int
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
        };
        auto default_operation = [&]()->int
        {
            options.assert_mutually_exclusive( "--value,--values" );
            std::vector< std::string > defaults;
            std::string default_value = options.value< std::string >( "--value", "" );
            if( default_value.empty() ) { defaults = comma::split( options.value< std::string >( "--values" ), ',' ); } // todo: use specified delimiter instead?
            std::string line;
            line.reserve( 4000 );
            while( std::cin.good() && !std::cin.eof() )
            {
                std::getline( std::cin, line );
                if( !line.empty() && *line.rbegin() == '\r' ) { line = line.substr( 0, line.length() - 1 ); } // windows... sigh...
                if( line.empty() ) { continue; }
                const std::vector< std::string >& values = comma::split( line, delimiter );
                std::string d;
                if( default_value.empty() )
                {
                    for( unsigned int i = 0; i < values.size(); d = delimiter, ++i ) { std::cout << d << ( values[i].empty() && i < defaults.size() ? defaults[i] : values[i] ); }
                }
                else
                {
                    for( unsigned int i = 0; i < values.size(); d = delimiter, ++i ) { std::cout << d << ( values[i].empty()  ? default_value : values[i] ); }
                }
                std::cout << std::endl;
            }
            return 0;
        };
        auto prefix = [&]()->int
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
        };
        auto rename = [&]()->int
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
        };
        auto strip = [&]()->int
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
        };
        auto cut = [&]()->int
        {
            options.assert_mutually_exclusive( "--except,--fields", "--empty" );
            bool except = options.exists( "--except" );
            bool empty = options.exists( "--empty" );
            const std::string& f = except ? options.value< std::string >( "--except", "" ) : options.value< std::string >( "--fields", "" );
            const std::vector< std::string >& s = comma::split( f, delimiter );
            while( std::cin.good() )
            {
                std::string line;
                std::string comma;
                std::getline( std::cin, line );
                if( line.empty() ) { continue; }
                const std::vector< std::string >& v = comma::split( line, delimiter );
                for( unsigned int i = 0; i < v.size(); ++i )
                {
                    if( empty )
                    {
                        if( v[i].empty() ) { continue; }
                    }
                    else
                    { 
                        if( except != ( !v[i].empty() && std::find( s.begin(), s.end(), v[i] ) != s.end() ) ) { continue; }
                    }
                    std::cout << comma << v[i]; comma = delimiter;
                }
                std::cout << std::endl;
            }
            return 0;
        };
        auto has = [&]()->int
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
        };
        auto make_fixed = [&]()->int
        {
            const std::vector< std::string >& values = comma::split( options.value< std::string >( "--values", "" ), ',', true );
            const unsigned int count = options.value< unsigned int >( "--count,--size", values.size() );
            if( count == 0 ) { std::cerr << "csv-fields: make-fixed: please specify either --count or --values" << std::endl; }
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
                    for( unsigned int i = v.size(); i < count; i++ ) { std::cout << delimiter << ( i < values.size() ? values[i] : std::string() ); }
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
        };
        std::unordered_map< std::string, std::function< int() > > operations = { { "clear", clear }
                                                                               , { "cut", cut }
                                                                               , { "default", default_operation }
                                                                               , { "has", has }
                                                                               , { "make-fixed", make_fixed }
                                                                               , { "numbers", numbers }
                                                                               , { "prefix", prefix }
                                                                               , { "rename", rename }
                                                                               , { "strip", strip } };
        auto o = operations.find( operation );
        if( o != operations.end() ) { return o->second(); }
        std::cerr << "csv-fields: expected operation, got '" << operation << "'" << std::endl;
        return 1;
    }
    catch( std::exception& ex ) { std::cerr << "csv-fields: " << ex.what() << std::endl; }
    catch( ... ) { std::cerr << "csv-fields: unknown exception" << std::endl; }
    return 1;
}

