// Copyright (c) 2011 The University of Sydney
// Copyright (c) 2020 Vsevolod Vlaskine

/// @author vsevolod vlaskine

#include <iostream>
#include <map>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <boost/optional.hpp>
#include "../../application/command_line_options.h"
#include "../../base/none.h"
#include "../../string/string.h"
#include "../../xpath/xpath.h"

static void usage( bool )
{
    std::cerr << std::endl;
    std::cerr << "take path-value data on stdin, output as csv to stdout" << std::endl;
    std::cerr << std::endl;
    std::cerr << "usage: cat data.path-value | name-value-convert [<options>] > data.csv" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options" << std::endl;
    std::cerr << "    --delimiter,-d=<delimiter>; default=','; csv delimiter" << std::endl;
    std::cerr << "    --equal-sign,-e=<equal_sign>; default='='; equal sign" << std::endl;
    std::cerr << "    --fields,-f=<fields>; fields to output" << std::endl;
    std::cerr << "    --map,--dict; prefixed paths are a map, not list; expects input sorted by path" << std::endl;
    std::cerr << "                  run example below to make sense of it" << std::endl;
    std::cerr << "    --prefix,--path,-p=[<prefix>]; optional prefix" << std::endl;
    std::cerr << "    --unindexed-fields=<fields>; if no --fields specified, output unindexed fields once, if --fields specified, append given unindexed fields to all records" << std::endl;
    std::cerr << "    --unindexed-stream,--stream; read a stream of key-value pairs, on every input record output csv record with the field value set and other fields empty, see example below" << std::endl;
    std::cerr << "    --unindexed-stream-update,--update; read a stream of key-value pairs, on every input record output all up-to-date values of fields present in --unindexed-fields, see example below" << std::endl;
    std::cerr << "    --unquote; unquote string values" << std::endl;
    std::cerr << "    --unsorted; the input data is not sorted by index" << std::endl;
    std::cerr << std::endl;
    std::cerr << "examples" << std::endl;
    std::cerr << "    indexed data" << std::endl;
    std::cerr << "        cat <<EOF | name-value-to-csv --fields=name,value,status --prefix=my/test" << std::endl;
    std::cerr << "        my/test[0]/name=a" << std::endl;
    std::cerr << "        my/test[0]/value=10" << std::endl;
    std::cerr << "        my/test[0]/status=0" << std::endl;
    std::cerr << "        my/test[1]/name=b" << std::endl;
    std::cerr << "        my/test[1]/value=20" << std::endl;
    std::cerr << "        my/test[1]/status=1" << std::endl;
    std::cerr << "        EOF" << std::endl;
    std::cerr << std::endl;
    std::cerr << "        yields:" << std::endl;
    std::cerr << "            a,10,0" << std::endl;
    std::cerr << "            b,20,1" << std::endl;
    std::cerr << std::endl;
    std::cerr << "        cat <<EOF | name-value-to-csv --map --fields=name,value,status --prefix=my/test" << std::endl;
    std::cerr << "        my/test/x/name=a" << std::endl;
    std::cerr << "        my/test/x/value=10" << std::endl;
    std::cerr << "        my/test/x/status=0" << std::endl;
    std::cerr << "        my/test/y/name=b" << std::endl;
    std::cerr << "        my/test/y/status=1" << std::endl;
    std::cerr << "        EOF" << std::endl;
    std::cerr << std::endl;
    std::cerr << "        yields:" << std::endl;
    std::cerr << "            x,a,10,0" << std::endl;
    std::cerr << "            y,b,,1" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    unindexed fields" << std::endl;
    std::cerr << "        todo" << std::endl;
    std::cerr << "    unindexed fields with --unindexed-stream" << std::endl;
    std::cerr << "        > ( echo a=1; echo c=3; echo b=3; echo a=2; echo b=4 ) | name-value-to-csv --unindexed-fields a,b --unindexed-stream" << std::endl;
    std::cerr << "        1," << std::endl;
    std::cerr << "        ,3" << std::endl;
    std::cerr << "        2," << std::endl;
    std::cerr << "        ,4" << std::endl;
    std::cerr << "    unindexed fields with --unindexed-stream-update" << std::endl;
    std::cerr << "        > ( echo a=1; echo c=3; echo b=3; echo a=2; echo b=4 ) | name-value-to-csv --unindexed-fields a,b --unindexed-stream" << std::endl;
    std::cerr << "        1," << std::endl;
    std::cerr << "        1,3" << std::endl;
    std::cerr << "        2,3" << std::endl;
    std::cerr << "        2,4" << std::endl;
    std::cerr << std::endl;
    exit( 0 );
}

typedef std::unordered_map< std::string, std::string > values_t;

static std::string join( const std::vector< std::string >& fields, values_t& values, char delimiter, bool clear = true )
{
    std::ostringstream oss;
    std::string comma;
    for( const auto& f: fields ) { oss << comma << values[f]; comma = delimiter; } // quick and dirty as everything else
    if( clear ) { values.clear(); }
    return oss.str();
}

int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av, usage );
        options.assert_mutually_exclusive( "--fields", "--unindexed-stream,--stream,--unindexed-stream-update,--update" );
        std::string fs = options.value< std::string >( "--fields,-f", "" );
        std::vector< std::string > fields = comma::split( fs, ',' );
        std::vector< std::string > unindexed_fields;
        std::string ufs = options.value< std::string >( "--unindexed-fields", "" );
        bool unindexed_stream_update = options.exists( "--unindexed-stream-update,--update" );
        bool unindexed_stream = options.exists( "--unindexed-stream,--stream" ) || unindexed_stream_update;
        std::unordered_set< std::string > unindexed_fields_set;
        if( !ufs.empty() )
        { 
            unindexed_fields = comma::split( ufs, ',' );
            for( const auto& f: unindexed_fields ) { unindexed_fields_set.insert( f ); }
        }
        if( fields[0].empty() && unindexed_fields.empty() ) { std::cerr << "name-value-to-csv: please specify --fields or --unindexed-fields" << std::endl; return 1; }
        bool unindexed = fields[0].empty();
        values_t unindexed_values;
        bool unquote = options.exists( "--unquote" );
        bool unsorted = options.exists( "--unsorted" );
        char delimiter = options.value( "--delimiter,-d", ',' );
        char equal_sign = options.value( "--equal-sign,-e", '=' );
        std::string prefix = options.value< std::string >( "--prefix,--path,-p", "" );
        values_t values; // quick and dirty; watch performance?
        std::map< unsigned int, values_t > map;
        boost::optional< unsigned int > index{ comma::silent_none< unsigned int >() };
        std::string key;
        bool is_map = options.exists( "--dict,--map" );
        if( is_map && unsorted ) { comma::say() << "combination of --map and --unsorted: todo, just ask" << std::endl; return 1; }
        std::string::size_type e = std::string::npos;
        auto value = [&]( const std::string& s ) { const std::string& t = s.substr( e + 1 ); return unquote && t.size() >= 2 ? comma::strip( t, "\"" ) : t; };
        while( std::cin.good() && !std::cin.eof() )
        {
            std::string s;
            std::getline( std::cin, s );
            if( comma::strip( s, " \t" ).empty() || comma::strip( s, " \t" )[0] == '#' ) { continue; }
            e = s.find_first_of( equal_sign ); // todo: use boost::spirit
            if( e == std::string::npos ) { std::cerr << "name-value-to-csv: expected path-value pair; got: '" << s << "'" << std::endl; return 1; }
            std::string name = s.substr( 0, e );
            if( unindexed_fields_set.find( name ) != unindexed_fields_set.end() )
            {
                unindexed_values[name] = value( s );
                if( unindexed_stream ) { std::cout << join( unindexed_fields, unindexed_values, delimiter, !unindexed_stream_update ) << std::endl; }
                continue;
            }
            if( name.substr( 0, prefix.size() ) != prefix ) { continue; }
            if( unindexed )
            {
                if( prefix.empty() ) { values[name] = value( s ); }
                else if( name[ prefix.size() ] == '/' ) { values[ name.substr( prefix.size() + 1 ) ] = value( s ); }
                continue;
            }
            if( is_map )
            {
                if( name[prefix.size()] != '/' ) { continue; }
                auto b = s.find_first_of( '/', prefix.size() + 1 );
                if( b == std::string::npos ) { std::cerr << "name-value-to-csv: with prefix \"" << prefix << "\" expected path-value pair with valid keys; got: '" << s << "'" << std::endl; return 1; }
                std::string current_key = name.substr( prefix.size() + 1, b - prefix.size() - 1 );
                if( unsorted ) {} // todo
                if( !key.empty() && current_key != key ) { std::cout << key << delimiter << join( fields, values, delimiter ) << std::endl; }
                values[name.substr( b + 1 )] = value( s );
                key = current_key;
            }
            else
            {
                if( name[prefix.size()] != '[' ) { continue; }
                auto b = s.find_first_of( ']', prefix.size() );
                if( b == std::string::npos ) { std::cerr << "name-value-to-csv: with prefix \"" << prefix << "\" expected path-value pair with valid indices; got: '" << s << "'" << std::endl; return 1; }
                if( s[ b + 1 ] != '/' ) { continue; }
                unsigned int current_index = boost::lexical_cast< unsigned int >( name.substr( prefix.size() + 1, b - prefix.size() - 1 ) );
                if( unsorted || !unindexed_fields.empty() ) { map[current_index][name.substr( b + 2 )] = value( s ); continue; }
                if( index && current_index < *index ) { std::cerr << "name-value-to-csv: expected sorted index, got index " << current_index << " after " << *index << " in line: '" << comma::strip( s ) << "'" << std::endl; return 1; }
                if( index && current_index > *index ) { std::cout << join( fields, values, delimiter ) << std::endl; }
                values[name.substr( b + 2 )] = value( s );
                index = current_index;
            }
        }
        if( unindexed && !unindexed_stream )
        { 
            std::cout << join( unindexed_fields, unindexed_values, delimiter ) << std::endl;
        }
        else if( unsorted || !unindexed_fields.empty() )
        { 
            std::string u;
            if( !unindexed_fields.empty() ) { u = join( unindexed_fields, unindexed_values, delimiter ); }
            for( auto v: map )
            {
                std::cout << join( fields, v.second, delimiter );
                if( !unindexed_fields.empty() ) { std::cout << delimiter << u; }
                std::cout << std::endl;
            }
        }
        else if( index )
        {
            std::cout << join( fields, values, delimiter ) << std::endl;
        }
        else if( is_map && !key.empty() )
        {
            std::cout << key << delimiter << join( fields, values, delimiter ) << std::endl;
        }
        return 0;
    }
    catch( std::exception& ex ) { std::cerr << "name-value-to-csv: " << ex.what() << std::endl; }
    catch( ... ) { std::cerr << "name-value-to-csv: unknown exception" << std::endl; }
    return 1;
}
