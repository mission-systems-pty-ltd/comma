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

#include <unordered_map>
#include "../../application/command_line_options.h"
#include "../../base/exception.h"
#include "../../base/types.h"
#include "../../csv/stream.h"
#include "../../csv/impl/unstructured.h"
#include "../../string/string.h"

static void usage( bool verbose )
{
    std::cerr << R"(
append unique id to csv records with the same values; support integer, time, and string fields

usage: cat data.csv | csv-enumerate <options>

todo: support floating point values as input keys

options
    --fields,-f=<fields>; fields of interest, actual field names do not matter
                          e.g: --fields ,,,a,,b,,,c
    --format=<binary format>; if input is ascii and deducing data types may be ambiguous,
                              define field types explicitly, value as in --binary
    --output-map,--map: do not output input records, only an unsorted list of keys
                        output fields
                            - list of input key values; in same binary as input
                            - corresponding enumeration index as ui
                            - number of values for this enumeration index as ui
    --verbose,-v: more output to stderr
)" << std::endl;
std::cerr << "csv options" << std::endl;
    std::cerr << comma::csv::options::usage( verbose ) << std::endl;
    exit( 0 );
}

struct output
{
    comma::uint32 id;
    output( comma::uint32 id = 0 ): id( id ) {}
};

namespace comma { namespace visiting {

template <> struct traits< output >
{
    template < typename K, typename V > static void visit( const K&, const output& p, V& v ) { v.apply( "id", p.id ); }
    template < typename K, typename V > static void visit( const K&, output& p, V& v ) { v.apply( "id", p.id ); }
};

} } // namespace comma { namespace visiting {

int main( int ac, char** av )
{
    typedef comma::csv::impl::unstructured input_t;
    typedef std::unordered_map< comma::csv::impl::unstructured, std::pair< comma::uint32, comma::uint32 >, comma::csv::impl::unstructured::hash >  map_t;
    try
    {
        comma::command_line_options options( ac, av, usage );
        bool output_map = options.exists( "--output-map,--map" );
        comma::csv::options csv( options );
        bool has_non_empty_field = false;
        for( const auto& f: comma::split( csv.fields, ',' ) ) { if( !f.empty() ) { has_non_empty_field = true; break; } }
        COMMA_ASSERT_BRIEF( has_non_empty_field, "please specify at least one key in fields" );
        std::string first_line;
        comma::csv::format f;
        if( csv.binary() ) { f = csv.format(); }
        else if( options.exists( "--format" ) ) { f = comma::csv::format( options.value< std::string >( "--format" ) ); }
        else
        {
            while( std::cin.good() && first_line.empty() ) { std::getline( std::cin, first_line ); }
            if( first_line.empty() ) { return 0; }
            f = comma::csv::impl::unstructured::guess_format( first_line, csv.delimiter );
            comma::saymore() << "guessed format: " << f.string() << std::endl;
        }
        input_t default_input;
        std::vector< std::string > v = comma::split( csv.fields, ',' );
        std::vector< std::string > format; // quick and dirty
        std::vector< std::string > s;
        if( csv.binary() ) { format = comma::split( csv.format().expanded_string(), ',' ); }
        for( unsigned int i = 0; i < v.size(); ++i )
        {
            if( v[i].empty() ) { continue; }
            v[i] = default_input.append( f.offset( i ).type );
            if( csv.binary() ) { s.push_back( format[i] ); }
        }
        std::string map_output_binary_format = comma::join( s, ',' );
        comma::saymore() << "fields " << csv.fields << " interpreted as: " << comma::join( v, ',' ) << std::endl;
        csv.fields = comma::join( v, ',' );
        static map_t map;
        comma::uint32 id = 0;
        if( !first_line.empty() )
        { 
            input_t input = comma::csv::ascii< input_t >( csv, default_input ).get( first_line );
            map[ comma::csv::ascii< input_t >( csv, default_input ).get( first_line ) ] = std::make_pair( id++, 1 );
            if( !output_map ) { std::cout << first_line << csv.delimiter << 0 << std::endl; }
        }
        comma::csv::options output_csv;
        output_csv.delimiter = csv.delimiter;
        if( csv.binary() ) { output_csv.format( comma::csv::format::value< output >() ); }
        comma::csv::input_stream< input_t > istream( std::cin, csv, default_input );
        comma::csv::output_stream< output > ostream( std::cout, output_csv );
        comma::csv::tied< input_t, output > tied( istream, ostream );
        #ifdef WIN32
        if( istream.is_binary() ) { _setmode( _fileno( stdout ), _O_BINARY ); }
        #endif
        while( istream.ready() || std::cin.good() )
        {
            const input_t* p = istream.read();
            if( !p ) { break; }
            map_t::iterator it = map.find( *p );
            comma::uint32 cur = id;
            if( it == map.end() ) { map[ *p ] = std::make_pair( id++, 1 ); } else { cur = it->second.first; ++( it->second.second ); }
            if( !output_map ) { tied.append( output( cur ) ); }
        }
        if( !output_map ) { return 0; }
        comma::csv::options output_map_csv;
        output_map_csv.delimiter = csv.delimiter;
        if( csv.binary() )
        { 
            output_map_csv.format( map_output_binary_format + ",2ui" ); //output_map_csv.format( comma::csv::format::value< input_t >( default_input ) + ",2ui" );
            comma::say() << "binary output format for map: '" << output_map_csv.format().string() << "'" << std::endl;
        }
        comma::csv::output_stream< map_t::value_type > omstream( std::cout, output_map_csv, std::make_pair( default_input, std::make_pair( 0, 0 ) ) );
        for( map_t::const_iterator it = map.begin(); it != map.end(); ++it ) { omstream.write( *it ); }
        return 0;
    }
    catch( std::exception& ex ) { comma::say() << ex.what() << std::endl; }
    catch( ... ) { comma::say() << "unknown exception" << std::endl; }
    return 1;
}
