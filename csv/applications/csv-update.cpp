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

#include <string.h>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <boost/array.hpp>
#include <boost/functional/hash.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/unordered_map.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <comma/application/command_line_options.h>
#include <comma/application/contact_info.h>
#include <comma/application/signal_flag.h>
#include <comma/base/types.h>
#include <comma/csv/stream.h>
#include <comma/csv/impl/unstructured.h>
#include <comma/io/stream.h>
#include <comma/string/string.h>
#include <comma/visiting/traits.h>

static void usage( bool more )
{
    std::cerr << std::endl;
    std::cerr << "todo" << std::endl;
    std::cerr << std::endl;
    std::cerr << "update ... csv files or streams by one or several keys (integer only for now)" << std::endl;
    std::cerr << std::endl;
    std::cerr << "usage: cat something.csv | csv-update \"update.csv\" [<options>] > updated.csv" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    fields:" << std::endl;
    std::cerr << "        block: block number" << std::endl;
    std::cerr << "        id: key to match, multiple id fields allowed" << std::endl;
    std::cerr << "        any other field names: fields to update, if none given, update " << std::endl;
    std::cerr << "                               all the non-id fields" << std::endl;
    std::cerr << "                               todo: only ascii supported, binary: to implement" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options:" << std::endl;
    std::cerr << "    --help,-h: help; --help --verbose: more help" << std::endl;
    std::cerr << "    --empty=<field values>; what field value stands for an empty field" << std::endl;
    std::cerr << "        ascii: default: empty string" << std::endl;
    std::cerr << "        binary: todo: no reasonable default" << std::endl;
    std::cerr << "        e.g: --empty=,,empty,,0: for the 3rd field, \"empty\" indicates it has empty value, for the 5th: 0" << std::endl;
    std::cerr << "    --format=<format>; in ascii mode, a hint of data format, e.g. --format=3ui,2d" << std::endl;
    std::cerr << "    --matched-only,--matched,-m: output only updates present on stdin" << std::endl;
    std::cerr << "    --remove,--reset,--unset=<field values>; what field value indicates that previous value should be replaced with empty value" << std::endl;
    std::cerr << "        e.g: --remove=,,remove,,: for the 3rd field, \"empty\" indicates it has empty value, for the 5th: 0" << std::endl;
    std::cerr << "    --update-non-empty-fields,--update-non-empty,-u:" << std::endl;
    std::cerr << "        ascii: if update has empty fields, keep the fields values from stdin" << std::endl;
    std::cerr << "        binary: todo, since the semantics of an \"empty\" value is unclear" << std::endl;
    std::cerr << "    --verbose,-v: more output to stderr" << std::endl;
    if( more ) { std::cerr << std::endl << "csv options:" << std::endl << comma::csv::options::usage() << std::endl; }
    std::cerr << std::endl;
    std::cerr << "examples" << std::endl;
    std::cerr << "    single key" << std::endl;
    std::cerr << "        cat entries.csv | csv-update updates.csv --fields=id" << std::endl;
    std::cerr << "    multiple keys" << std::endl;
    std::cerr << "        cat entries.csv | csv-update updates.csv --fields=id,id" << std::endl;
    std::cerr << "    keys are strings" << std::endl;
    std::cerr << "        cat entries.csv | csv-update updates.csv --fields=id --string" << std::endl;
    std::cerr << "    output only matched entries from update.csv" << std::endl;
    std::cerr << "        cat entries.csv | csv-update updates.csv --fields=id --matched-only" << std::endl;
    std::cerr << "    update only non-empty fields in update.csv" << std::endl;
    std::cerr << "    e.g. if an entry in update.csv is: 0,,1 only the 1st and 3rd fields will be updated" << std::endl;
    std::cerr << "        cat entries.csv | csv-update updates.csv --fields=id --update-non-empty-fields" << std::endl;
    std::cerr << std::endl;
    std::cerr << comma::contact_info << std::endl;
    std::cerr << std::endl;
    exit( -1 );
}

struct input_t
{
    comma::csv::impl::unstructured key;
    comma::csv::impl::unstructured value;
    comma::uint32 block;
    
    input_t() : block( 0 ) {}
    input_t( comma::csv::impl::unstructured key, comma::csv::impl::unstructured value, comma::uint32 block ): key( key ), value( value ), block( block ) {}
    
    typedef boost::unordered_map< comma::csv::impl::unstructured, std::vector< comma::csv::impl::unstructured >, comma::csv::impl::unstructured::hash > map_t;
    typedef comma::csv::input_stream< input_t > input_stream_t;
};

namespace comma { namespace visiting {

template <> struct traits< input_t >
{
    template < typename K, typename V > static void visit( const K&, const input_t& p, V& v )
    {
        v.apply( "key", p.key );
        v.apply( "value", p.value );
        v.apply( "block", p.block );
    }
    template < typename K, typename V > static void visit( const K&, input_t& p, V& v )
    {
        v.apply( "key", p.key );
        v.apply( "value", p.value );
        v.apply( "block", p.block );
    }
};

} } // namespace comma { namespace visiting {

static bool verbose;
static comma::csv::options csv;
static std::string filter_name;
static boost::scoped_ptr< comma::io::istream > filter_transport;
static comma::signal_flag is_shutdown;
static comma::uint32 block = 0;
static bool matched_only = false;
static bool update_non_empty = false;
static input_t default_input;
static comma::csv::impl::unstructured empty;
static comma::csv::impl::unstructured erase; // todo
static input_t::map_t filter_map;
static input_t::map_t unmatched;

static void output_unmatched( comma::csv::output_stream< input_t >& ostream )
{
    if( !is_shutdown && !matched_only )
    {
        for( typename input_t::map_t::const_iterator it = unmatched.begin(); it != unmatched.end(); ++it )
        {
            for( std::size_t i = 0; i < it->second.size(); ++i ) // quick and dirty
            {
                ostream.write( input_t( it->first, it->second[i], block ) );
            }
        }
    }
    unmatched.clear();
}

static void read_filter_block( comma::csv::output_stream< input_t >& ostream )
{
    output_unmatched( ostream );
    static input_t::input_stream_t filter_stream( **filter_transport, csv, default_input );
    static const input_t* last = filter_stream.read();
    if( !last ) { return; }
    block = last->block;
    filter_map.clear();
    comma::uint64 count = 0;
    while( last->block == block && !is_shutdown )
    {
        filter_map[ last->key ].push_back( last->value );
        //if( d.size() > 1 ) {}
        if( verbose ) { ++count; if( count % 10000 == 0 ) { std::cerr << "csv-update: reading block " << block << "; loaded " << count << " point[s]; hash map size: " << filter_map.size() << std::endl; } }
        //if( ( *filter_transport )->good() && !( *filter_transport )->eof() ) { break; }
        last = filter_stream.read();
        if( !last ) { break; }
    }
    unmatched = filter_map;
    if( verbose ) { std::cerr << "csv-update: read block " << block << " of " << count << " point[s]; got " << filter_map.size() << " different key(s)" << std::endl; }
}

static void output_last( const comma::csv::input_stream< input_t >& istream )
{
    if( istream.is_binary() ) { std::cout.write( istream.binary().last(), csv.format().size() ); }
    else { std::cout << comma::join( istream.ascii().last(), csv.delimiter ) << std::endl; }
}

static void update( comma::csv::impl::unstructured& value, const comma::csv::impl::unstructured& value_update, bool non_empty_only )
{
    if( !non_empty_only ) { value = value_update; return; }
    for( std::size_t i = 0; i < value.longs.size(); ++i ) { if( value_update.longs[i] != empty.longs[i] ) { value.longs[i] = value_update.longs[i]; } }
    for( std::size_t i = 0; i < value.strings.size(); ++i ) { if( value_update.strings[i] != empty.strings[i] ) { value.strings[i] = value_update.strings[i]; } }
    for( std::size_t i = 0; i < value.doubles.size(); ++i ) { if( value_update.doubles[i] != empty.doubles[i] ) { value.doubles[i] = value_update.doubles[i]; } }
    for( std::size_t i = 0; i < value.time.size(); ++i ) { if( value_update.time[i] != empty.time[i] ) { value.time[i] = value_update.time[i]; } }
    // todo: handle erase value
}

static void update( const input_t& v, const comma::csv::input_stream< input_t >& istream, comma::csv::output_stream< input_t >& ostream, const std::string& last = std::string() )
{
    typename input_t::map_t::const_iterator it = filter_map.find( v.key );
    if( it == filter_map.end() || it->second.empty() )
    { 
        if( last.empty() ) { output_last( istream ); }
        else { std::cout << last << std::endl; }
        return;
    }
    input_t current = v;
    for( std::size_t i = 0; i < it->second.size(); ++i )
    {
        update( current.value, it->second[i], update_non_empty );
        ostream.write( current, istream ); // todo: output last only
    }
    unmatched.erase( it->first );
}

int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av, usage );
        verbose = options.exists( "--verbose,-v" );
        csv = comma::csv::options( options );
        csv.full_xpath = true;
        csv.quote.reset();
        matched_only = options.exists( "--matched-only,--matched,-m" );
        update_non_empty = options.exists( "--update-non-empty-fields,--update-non-empty,-u" );
        if( csv.binary() && update_non_empty ) { std::cerr << "csv-update: --update-non-empty-fields in binary mode not supported" << std::endl; return 1; }
        std::vector< std::string > unnamed = options.unnamed( "--matched-only,--matched,-m,--string,-s,--update-non-empty-fields,--update-non-empty,-u,--verbose,-v", "-.*" );
        if( unnamed.empty() ) { std::cerr << "csv-update: please specify the second source" << std::endl; return 1; }
        if( unnamed.size() > 1 ) { std::cerr << "csv-update: expected one file or stream to join, got " << comma::join( unnamed, ' ' ) << std::endl; return 1; }
        filter_name = unnamed[0];
        std::vector< std::string > v = comma::split( csv.fields, ',' );
        bool has_value_fields = false;
        for( std::size_t i = 0; !has_value_fields && i < v.size(); has_value_fields = !v[i].empty() && v[i] != "block" &&  v[i] != "id", ++i );
        std::string first_line;
        comma::csv::format f;
        if( csv.binary() ) { f = csv.format(); }
        else if( options.exists( "--format" ) ) { f = comma::csv::format( options.value< std::string >( "--format" ) ); }
        else
        {
            while( std::cin.good() && first_line.empty() ) { std::getline( std::cin, first_line ); }
            if( first_line.empty() ) { return 0; }
            f = comma::csv::impl::unstructured::guess_format( first_line, csv.delimiter );
            std::cerr << "csv-update: guessed format: " << f.string() << std::endl;
        }
        unsigned int size = f.count();
        for( std::size_t i = 0; i < size; ++i )
        {
            if( i < v.size() )
            {
                if( v[i] == "block" ) { continue; }
                if( v[i] == "id" ) { v[i] = "key/" + default_input.key.append( f.offset( i ).type ); continue; }
            }
            if( !has_value_fields || !v[i].empty() )
            {
                v.resize( size );
                v[i] = "value/" + default_input.value.append( csv.binary() ? f.offset( i ).type : comma::csv::format::fixed_string ); // quick and dirty
            }
        }
        if( default_input.key.empty() ) { std::cerr << "csv-update: please specify at least one id field" << std::endl; return 1; }
        csv.fields = comma::join( v, ',' );
        if( verbose ) { std::cerr << "csv-update: csv fields: " << csv.fields << std::endl; }
        comma::csv::input_stream< input_t > istream( std::cin, csv, default_input );
        filter_transport.reset( new comma::io::istream( filter_name, csv.binary() ? comma::io::mode::binary : comma::io::mode::ascii ) );
        comma::csv::output_stream< input_t > ostream( std::cout, csv, default_input );
        empty = default_input.value;
        
        
        // todo: handle --empty
        
        
        for( unsigned int i = 0; i < empty.longs.size(); ++i ) { empty.longs[i] = std::numeric_limits< comma::int64 >::max(); } // quick and dirty
        for( unsigned int i = 0; i < empty.doubles.size(); ++i ) { empty.doubles[i] = std::numeric_limits< double >::max(); } // quick and dirty
        default_input.value = empty;
        read_filter_block( ostream );
        if( !first_line.empty() ) { update( comma::csv::ascii< input_t >( csv, default_input ).get( first_line ), istream, ostream, first_line ); }
        while( !is_shutdown && ( istream.ready() || ( std::cin.good() && !std::cin.eof() ) ) )
        {
            const input_t* p = istream.read();
            if( !p ) { break; }
            if( block != p->block ) { read_filter_block( ostream ); }
            update( *p, istream, ostream );
        }
        output_unmatched( ostream );
        return 0;
    }
    catch( std::exception& ex ) { std::cerr << "csv-update: " << ex.what() << std::endl; }
    catch( ... ) { std::cerr << "csv-update: unknown exception" << std::endl; }
    return 1;
}
