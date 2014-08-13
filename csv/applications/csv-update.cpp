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
#include <comma/application/command_line_options.h>
#include <comma/application/contact_info.h>
#include <comma/application/signal_flag.h>
#include <comma/base/types.h>
#include <comma/csv/stream.h>
#include <comma/io/stream.h>
#include <comma/string/string.h>
#include <comma/visiting/traits.h>

static void usage( bool more )
{
    std::cerr << std::endl;
    std::cerr << "todo" << std::endl;
    exit( 1 );
    
    std::cerr << std::endl;
    std::cerr << "join two csv files or streams by one or several keys (integer only for now)" << std::endl;
    std::cerr << std::endl;
    std::cerr << "usage: cat something.csv csv-update \"something_else.csv[,options]\" [<options>]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    fields:" << std::endl;
    std::cerr << "        block: block number" << std::endl;
    std::cerr << "        any other field names: keys" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options:" << std::endl;
    std::cerr << "    --help,-h: help; --help --verbose: more help" << std::endl;
    std::cerr << "    --first-matching: output only the first matching record (a bit of hack for now, but we needed it)" << std::endl;
    std::cerr << "    --not-matching: not matching records as read from stdin, no join performed" << std::endl;
    std::cerr << "    --string,-s: keys are strings; a quick and dirty option to support strings" << std::endl;
    std::cerr << "                 default: integers" << std::endl;
    std::cerr << "    --strict: fail, if id on stdin is not found" << std::endl;
    std::cerr << "    --verbose,-v: more output to stderr" << std::endl;
    if( more )
    {
        std::cerr << std::endl;
        std::cerr << "csv options:" << std::endl;
        std::cerr << comma::csv::options::usage() << std::endl;
    }
    std::cerr << std::endl;
    std::cerr << "examples:" << std::endl;
    std::cerr << "    todo" << std::endl;
    std::cerr << std::endl;
    std::cerr << comma::contact_info << std::endl;
    std::cerr << std::endl;
    exit( -1 );
}

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
        for( std::size_t i = 0; i < keys.size(); ++i ) { if( keys[i] < rhs.keys[i] ) { return true; } }
        return false;
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

    typedef boost::unordered_map< input, std::vector< std::string >, hash > filter_map;
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

static bool verbose;
static comma::csv::options csv;
boost::scoped_ptr< comma::io::istream > filter_transport;
comma::signal_flag is_shutdown;
static comma::uint32 block = 0;

template < typename K > struct join_impl_ // quick and dirty
{
    static typename input< K >::filter_map filter_map;
    
    static typename input< K >::filter_map unmatched;
    
    static input< K > default_input;

    static void read_filter_block()
    {
        static comma::csv::input_stream< input< K > > filter_stream( **filter_transport, csv, default_input );
        static const input< K >* last = filter_stream.read();
        if( !last ) { return; }
        block = last->block;
        filter_map.clear();
        comma::uint64 count = 0;
        while( last->block == block && !is_shutdown )
        {
            if( filter_stream.is_binary() )
            {
                typename input< K >::filter_map::mapped_type& d = filter_map[ *last ];
                d.push_back( std::string() );
                d.back().resize( csv.format().size() );
                ::memcpy( &d.back()[0], filter_stream.binary().last(), csv.format().size() );
            }
            else
            {
                filter_map[ *last ].push_back( comma::join( filter_stream.ascii().last(), csv.delimiter ) );
            }
            if( verbose ) { ++count; if( count % 10000 == 0 ) { std::cerr << "csv-update: reading block " << block << "; loaded " << count << " point[s]; hash map size: " << filter_map.size() << std::endl; } }
            //if( ( *filter_transport )->good() && !( *filter_transport )->eof() ) { break; }
            last = filter_stream.read();
            if( !last ) { break; }
        }
        unmatched = filter_map;
        if( verbose ) { std::cerr << "csv-update: read block " << block << " of " << count << " point[s]; hash map size: " << filter_map.size() << std::endl; }
    }

    static int run( const comma::command_line_options& options )
    {
        std::vector< std::string > v = comma::split( csv.fields, ',' );
        std::vector< std::string > w = comma::split( csv.fields, ',' );
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
        if( default_input.keys.empty() ) { std::cerr << "csv-update: please specify at least one common key" << std::endl; return 1; }
        csv.fields = comma::join( v, ',' );
        csv.fields = comma::join( w, ',' );
        comma::csv::input_stream< input< K > > stdin_stream( std::cin, csv, default_input );
        filter_transport.reset( new comma::io::istream( csv.filename, csv.binary() ? comma::io::mode::binary : comma::io::mode::ascii ) );
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
            if( filter_map.empty() ) { break; }
            typename input< K >::filter_map::const_iterator it = filter_map.find( *p );
            if( it == filter_map.end() || it->second.empty() )
            {
                std::string s;
                comma::csv::options c;
                c.fields = "keys";
                std::cerr << "csv-update: match not found for key(s): " << comma::csv::ascii< input< K > >( c, default_input ).put( *p, s ) << ", block: " << block << std::endl;
                return 1;
            }
            if( stdin_stream.is_binary() )
            {
                for( std::size_t i = 0; i < it->second.size(); ++i )
                {
                    std::cout.write( stdin_stream.binary().last(), csv.format().size() );
                    std::cout.write( &( it->second[i][0] ), csv.format().size() );
                    std::cout.flush();
                }
                std::cout.flush();
            }
            else
            {
                for( std::size_t i = 0; i < it->second.size(); ++i )
                {
                    std::cout << comma::join( stdin_stream.ascii().last(), csv.delimiter ) << csv.delimiter;
                    std::cout << ( csv.binary()
                                 ? csv.format().bin_to_csv( &it->second[i][0], csv.delimiter )
                                 : it->second[i] ) << std::endl;
                }
            }
            unmatched.erase( it->first );
        }
        
        // todo: output unmatched
        
        if( verbose ) { std::cerr << "csv-update: discarded " << discarded << " entrie[s] with no matches" << std::endl; }
        return 0;
    }
};

template < typename K > typename input< K >::filter_map join_impl_< K >::unmatched;
template < typename K > typename input< K >::filter_map join_impl_< K >::filter_map;
template < typename K > input< K > join_impl_< K >::default_input;

int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av, usage );
        verbose = options.exists( "--verbose,-v" );
        csv = comma::csv::options( options );
        std::vector< std::string > unnamed = options.unnamed( "--verbose,-v,--string", "-.*" );
        if( unnamed.empty() ) { std::cerr << "csv-update: please specify the second source" << std::endl; return 1; }
        if( unnamed.size() > 1 ) { std::cerr << "csv-update: expected one file or stream to join, got " << comma::join( unnamed, ' ' ) << std::endl; return 1; }
        return options.exists( "--string,-s" ) ? join_impl_< std::string >::run( options ) : join_impl_< comma::int64 >::run( options );
    }
    catch( std::exception& ex ) { std::cerr << "csv-update: " << ex.what() << std::endl; }
    catch( ... ) { std::cerr << "csv-update: unknown exception" << std::endl; }
    return 1;
}
