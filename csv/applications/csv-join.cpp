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

#include <string.h>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <boost/array.hpp>
#include <boost/functional/hash.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/unordered_map.hpp>
#include <comma/application/command_line_options.h>
#include <comma/application/contact_info.h>
#include <comma/application/signal_flag.h>
#include <comma/base/exception.h>
#include <comma/base/types.h>
#include <comma/csv/stream.h>
#include <comma/csv/traits.h>
#include <comma/io/stream.h>
#include <comma/math/compare.h>
#include <comma/name_value/parser.h>
#include <comma/string/string.h>
#include <comma/visiting/traits.h>

static void usage( bool more )
{
    std::cerr << std::endl;
    std::cerr << "Join two csv files or streams by one or several keys" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Usage: cat something.csv csv-join \"something_else.csv[,options]\" [<options>]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    fields:" << std::endl;
    std::cerr << "        block: block number" << std::endl;
    std::cerr << "        any other field names: keys" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Options:" << std::endl;
    std::cerr << "    --help,-h: help; --help --verbose: more help" << std::endl;
    std::cerr << "    --first-matching: output only the first matching record (a bit of hack for now, but we needed it)" << std::endl;
    std::cerr << "    --not-matching: not matching records as read from stdin, no join performed" << std::endl;
    std::cerr << "    --string,-s: keys are strings; a quick and dirty option to support strings" << std::endl;
    std::cerr << "                 default: integers" << std::endl;
    std::cerr << "    --strict: fail, if id on stdin is not found" << std::endl;
    std::cerr << "    --tolerance,--epsilon=<value>; compare keys with given tolerance" << std::endl;
    std::cerr << "    --verbose,-v: more output to stderr" << std::endl;
    if( more )
    {
        std::cerr << std::endl;
        std::cerr << "csv options:" << std::endl;
        std::cerr << comma::csv::options::usage() << std::endl;
    }
    std::cerr << std::endl;
    std::cerr << "examples (try them)" << std::endl;
    std::cerr << "    on the following data file:" << std::endl;
    std::cerr << "        echo 1,2,hello > data.csv" << std::endl;
    std::cerr << "        echo 1,3,hello >> data.csv" << std::endl;
    std::cerr << "        echo 3,4,world >> data.csv" << std::endl;
    std::cerr << "    join with a matching record" << std::endl;
    std::cerr << "        echo 1,blah | csv-join --fields=id \"data.csv;fields=id\"" << std::endl;
    std::cerr << "        echo 3,blah | csv-join --fields=id \"data.csv;fields=,id\"" << std::endl;
    std::cerr << "        echo 5,blah | csv-join --fields=id \"data.csv;fields=,id\"" << std::endl;
    std::cerr << "        echo 5,blah | csv-join --fields=id \"data.csv;fields=,id\" --not-matching" << std::endl;
    std::cerr << "        echo 5,blah | csv-join --fields=id \"data.csv;fields=,id\" --strict" << std::endl;
    std::cerr << "    join by key which is a string" << std::endl;
    std::cerr << "        echo 1,hello | csv-join --fields=id \"data.csv;fields=,,id\" --string" << std::endl;
    std::cerr << "        echo 1,world | csv-join --fields=id \"data.csv;fields=,,id\" --string" << std::endl;
    std::cerr << "        echo 1,blah | csv-join --fields=id \"data.csv;fields=,,id\" --string" << std::endl;
    std::cerr << "        echo 1,blah | csv-join --fields=id \"data.csv;fields=,,id\" --string --not-matching" << std::endl;
    std::cerr << "        echo 1,blah | csv-join --fields=id \"data.csv;fields=,,id\" --string --strict" << std::endl;
    std::cerr << std::endl;
    std::cerr << comma::contact_info << std::endl;
    std::cerr << std::endl;
    exit( -1 );
}

static bool verbose;
static bool first_matching;
static bool strict;
static bool not_matching;
static comma::csv::options stdin_csv;
static comma::csv::options filter_csv;
boost::scoped_ptr< comma::io::istream > filter_transport;
comma::signal_flag is_shutdown;
static comma::uint32 block = 0;
static boost::optional< double > tolerance;

template < typename K >
struct input
{
    std::vector< K > keys;

    comma::uint32 block;

    input() : block( 0 ) {}

    bool operator==( const input& rhs ) const
    {
        for( std::size_t i = 0; i < keys.size(); ++i ) { if( keys[i] != rhs.keys[i] ) { return false; } }
        return true;
    }

    bool operator<( const input& rhs ) const
    {
        if( keys.size() > 1 ) { COMMA_THROW( comma::exception, "if --tolerance given, expected one key, got: " << keys.size() ); }
        return comma::math::less( keys[0], rhs.keys[0], *tolerance );
    }

    struct hash : public std::unary_function< input, std::size_t >
    {
        std::size_t operator()( input const& p ) const
        {
            std::size_t seed = 0;
            for( std::size_t i = 0; i < p.keys.size(); ++i ) { boost::hash_combine( seed, p.keys[i] ); }
            return seed;
        }
    };

    typedef boost::unordered_map< input, std::vector< std::string >, hash > unordered_map;
    typedef std::map< input, std::vector< std::string > > map;
};

template < typename K, bool Strict = true > struct traits
{
    typedef typename input< K >::unordered_map map;
    typedef std::pair< typename map::const_iterator, typename map::const_iterator > pair;
    static pair find( map& m, const input< K >& k )
    {
        typename map::const_iterator it = m.find( k );
        if( it == m.end() ) { return std::make_pair( it, it ); }
        typename map::const_iterator end = it;
        ++end;
        return std::make_pair( it, end );
    }
};

template < typename K > struct traits< K, false >
{
    typedef typename input< K >::map map;
    //typedef std::pair< typename map::const_iterator, typename map::const_iterator > pair;
    typedef std::pair< typename map::iterator, typename map::iterator > pair;
    static pair find( map& m, const input< K >& k ) { return std::make_pair( m.lower_bound( k ), m.upper_bound( k ) ); }
};

namespace comma { namespace visiting {

template < typename T > struct traits< input< T > >
{
    template < typename K, typename V > static void visit( const K&, const input< T >& p, V& v )
    {
        v.apply( "keys", p.keys );
        v.apply( "block", p.block );
    }
    template < typename K, typename V > static void visit( const K&, input< T >& p, V& v )
    {
        v.apply( "keys", p.keys );
        v.apply( "block", p.block );
    }
};

} } // namespace comma { namespace visiting {

template < typename K, bool Strict = true > struct join_impl_ // quick and dirty
{
    static typename traits< K, Strict >::map filter_map;
    static input< K > default_input;

    static void read_filter_block()
    {
        static comma::csv::input_stream< input< K > > filter_stream( **filter_transport, filter_csv, default_input );
        static const input< K >* last = filter_stream.read();
        if( !last ) { return; }
        block = last->block;
        filter_map.clear();
        comma::uint64 count = 0;
        while( last->block == block && !is_shutdown )
        {
            if( filter_stream.is_binary() )
            {
                typename traits< K, Strict >::map::mapped_type& d = filter_map[ *last ];
                d.push_back( std::string() );
                d.back().resize( filter_csv.format().size() );
                ::memcpy( &d.back()[0], filter_stream.binary().last(), filter_csv.format().size() );
            }
            else
            {
                filter_map[ *last ].push_back( comma::join( filter_stream.ascii().last(), stdin_csv.delimiter ) );
            }
            if( verbose ) { ++count; if( count % 10000 == 0 ) { std::cerr << "csv-join: reading block " << block << "; loaded " << count << " point[s]; hash map size: " << filter_map.size() << std::endl; } }
            //if( ( *filter_transport )->good() && !( *filter_transport )->eof() ) { break; }
            last = filter_stream.read();
            if( !last ) { break; }
        }
        if( verbose ) { std::cerr << "csv-join: read block " << block << " of " << count << " point[s]; hash map size: " << filter_map.size() << std::endl; }
    }

    static int run( const comma::command_line_options& options )
    {
        std::vector< std::string > v = comma::split( stdin_csv.fields, ',' );
        std::vector< std::string > w = comma::split( filter_csv.fields, ',' );
        for( std::size_t i = 0; i < v.size(); ++i ) // quick and dirty, wasteful, but who cares
        {
            if( v[i].empty() || v[i] == "block" ) { continue; }
            for( std::size_t k = 0; k < w.size(); ++k )
            {
                if( v[i] != w[k] ) { continue; }
                v[i] = "keys[" + boost::lexical_cast< std::string >( default_input.keys.size() ) + "]";
                w[k] = "keys[" + boost::lexical_cast< std::string >( default_input.keys.size() ) + "]";
                default_input.keys.resize( default_input.keys.size() + 1 ); // quick and dirty
            }
        }
        if( default_input.keys.empty() ) { std::cerr << "csv-join: please specify at least one common key" << std::endl; return 1; }
        stdin_csv.fields = comma::join( v, ',' );
        filter_csv.fields = comma::join( w, ',' );
        comma::csv::input_stream< input< K > > stdin_stream( std::cin, stdin_csv, default_input );
        filter_transport.reset( new comma::io::istream( filter_csv.filename, filter_csv.binary() ? comma::io::mode::binary : comma::io::mode::ascii ) );
        std::size_t discarded = 0;
        read_filter_block();
        #ifdef WIN32
        if( stdin_stream.is_binary() ) { _setmode( _fileno( stdout ), _O_BINARY ); }
        #endif
        while( !is_shutdown && std::cin.good() && !std::cin.eof() )
        {
            const input< K >* p = stdin_stream.read();
            if( !p ) { break; }
            if( block != p->block ) { read_filter_block(); }
            typename traits< K, Strict >::pair pair = traits< K, Strict >::find( filter_map, *p );
            if( pair.first == filter_map.end() ) // if( it == filter_map.end() || it->second.empty() )
            {
                if( not_matching )
                {
                    if( stdin_stream.is_binary() ) { std::cout.write( stdin_stream.binary().last(), stdin_csv.format().size() ); }
                    else { std::cout << comma::join( stdin_stream.ascii().last(), stdin_csv.delimiter ) << std::endl; }
                    continue;
                }
                if( !strict ) { ++discarded; continue; }
                std::string s;
                comma::csv::options c;
                c.fields = "keys";
                std::cerr << "csv-join: match not found for key(s): " << comma::csv::ascii< input< K > >( c, default_input ).put( *p, s ) << ", block: " << block << std::endl;
                return 1;
            }
            if( not_matching ) { continue; }
            for( typename traits< K, Strict >::map::const_iterator it = pair.first; it != pair.second; ++it )
            {
                if( stdin_stream.is_binary() )
                {
                    for( std::size_t i = 0; i < ( first_matching ? 1 : it->second.size() ); ++i )
                    {
                        std::cout.write( stdin_stream.binary().last(), stdin_csv.format().size() );
                        std::cout.write( &( it->second[i][0] ), filter_csv.format().size() );
                        std::cout.flush();
                    }
                    std::cout.flush();
                }
                else
                {
                    for( std::size_t i = 0; i < ( first_matching ? 1 : it->second.size() ); ++i )
                    {
                        std::cout << comma::join( stdin_stream.ascii().last(), stdin_csv.delimiter ) << stdin_csv.delimiter;
                        std::cout << ( filter_csv.binary()
                                     ? filter_csv.format().bin_to_csv( &it->second[i][0], stdin_csv.delimiter )
                                     : it->second[i] ) << std::endl;
                    }
                }
                if( first_matching ) { break; }
            }
            if( first_matching ) { filter_map.erase( pair.first, pair.second ); }
        }
        if( verbose ) { std::cerr << "csv-join: discarded " << discarded << " entrie[s] with no matches" << std::endl; }
        return 0;
    }
};

template < typename K, bool Strict > typename traits< K, Strict >::map join_impl_< K, Strict >::filter_map;
template < typename K, bool Strict > input< K > join_impl_< K, Strict >::default_input;

int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av, usage );
        verbose = options.exists( "--verbose,-v" );
        first_matching = options.exists( "--first-matching" );
        strict = options.exists( "--strict" );
        not_matching = options.exists( "--not-matching" );
        tolerance = options.optional< double >( "--tolerance,--epsilon" );
        options.assert_mutually_exclusive( "--tolerance,--epsilon,--first-matching" );
        options.assert_mutually_exclusive( "--tolerance,--epsilon,--string,-s" );
        stdin_csv = comma::csv::options( options );
        std::vector< std::string > unnamed = options.unnamed( "--verbose,-v,--first-matching,--not-matching,--string,-s,--strict", "-.*" );
        if( unnamed.empty() ) { std::cerr << "csv-join: please specify the second source" << std::endl; return 1; }
        if( unnamed.size() > 1 ) { std::cerr << "csv-join: expected one file or stream to join, got " << comma::join( unnamed, ' ' ) << std::endl; return 1; }
        comma::name_value::parser parser( "filename", ';', '=', false );
        filter_csv = parser.get< comma::csv::options >( unnamed[0] );
        if( stdin_csv.binary() && !filter_csv.binary() ) { std::cerr << "csv-join: stdin stream binary and filter stream ascii: this combination is not supported" << std::endl; return 1; }
        return   options.exists( "--string,-s" )
               ? join_impl_< std::string, true >::run( options )
               : tolerance
               ? join_impl_< double, false >::run( options )
               : join_impl_< comma::int64, true >::run( options );
    }
    catch( std::exception& ex )
    {
        std::cerr << "csv-join: " << ex.what() << std::endl;
    }
    catch( ... )
    {
        std::cerr << "csv-join: unknown exception" << std::endl;
    }
    return 1;
}
