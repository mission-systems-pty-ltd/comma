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
    std::cerr << "    --string,-s: keys are strings; a quick and dirty option to support strings" << std::endl;
    std::cerr << "                 default: integers" << std::endl;
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
    
    typedef std::pair< comma::csv::impl::unstructured, std::string > record_t;
    typedef boost::unordered_map< comma::csv::impl::unstructured, std::vector< record_t > > map_t;
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
boost::scoped_ptr< comma::io::istream > filter_transport;
comma::signal_flag is_shutdown;
static comma::uint32 block = 0;
bool matched_only = false;
bool update_non_empty = false;

int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av, usage );
        verbose = options.exists( "--verbose,-v" );
        csv = comma::csv::options( options );
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
        input_t default_input;
        
        // todo: if ascii and no --format, guess format
        
        comma::csv::format f = csv.binary() ? csv.format() : comma::csv::format( options.value< std::string >( "--format" ) );
        for( std::size_t i = 0; i < v.size(); ++i )
        {
            if( v[i] == "block" ) { continue; }
            if( v[i] == "id" ) { v[i] = "key/" + default_input.key.append( f.offset( i ).type ); continue; }
            if( !v[i].empty() || !has_value_fields ) { v[i] = "value/" + default_input.value.append( f.offset( i ).type ); }
        }
        if( default_input.key.empty() ) { std::cerr << "csv-update: please specify at least one id field" << std::endl; return 1; }
        csv.fields = comma::join( v, ',' );
        
        
        // todo
       
        
        std::cerr << "---------------------------" << std::endl;
        std::cerr << "csv.fields: " << csv.fields << std::endl;
        std::cerr << "---------------------------" << std::endl;
        
        comma::csv::input_stream< input_t > istream( std::cin, csv, default_input );
        
        comma::csv::output_stream< input_t > ostream( std::cout, csv, default_input );
        
        
        return 0;
        
        
        // todo
        
        
//         input_stream_t stdin_stream( std::cin, csv, default_input );
//         output_stream_t ostream( std::cout, csv, default_input );
//         filter_transport.reset( new comma::io::istream( filter_name, csv.binary() ? comma::io::mode::binary : comma::io::mode::ascii ) );
//         read_filter_block( ostream );
//         #ifdef WIN32
//         if( stdin_stream.is_binary() ) { _setmode( _fileno( stdout ), _O_BINARY ); }
//         #endif
//         while( !is_shutdown && ( stdin_stream.ready() || ( std::cin.good() && !std::cin.eof() ) ) )
//         {
//             const input< K >* p = stdin_stream.read();
//             if( !p ) { break; }
//             if( block != p->key.block ) { read_filter_block( ostream ); }
//             typename input< K >::filter_map::const_iterator it = filter_map.find( p->key );
//             if( it == filter_map.end() || it->second.empty() ) { output_last( stdin_stream ); continue; }
//             input< K > current = *p;
//             for( std::size_t i = 0; i < it->second.size(); ++i )
//             {
//                 update( current.value, it->second[i], update_non_empty );
//                 ostream.write( current, stdin_stream ); // todo: output last only
//             }
//             unmatched.erase( it->first );
//         }
//         output_unmatched( ostream );
        return 0;
    }
    catch( std::exception& ex ) { std::cerr << "csv-update: " << ex.what() << std::endl; }
    catch( ... ) { std::cerr << "csv-update: unknown exception" << std::endl; }
    return 1;
}

































struct value_type
{
    std::vector< std::string > strings;
    //std::vector< boost::posix_time::ptime > time;
    //std::vector< double > doubles;
    //std::vector< comma::uint64 > integers; // todo?
    bool empty() const { return strings.empty(); }
};

template < typename K >
struct key_type
{
    std::vector< K > ids;

    comma::uint32 block;

    key_type() : block( 0 ) {}

    bool operator==( const key_type& rhs ) const
    {
        for( std::size_t i = 0; i < ids.size(); ++i ) { if( ids[i] != rhs.ids[i] ) { return false; } }
        return true;
    }

    bool operator<( const key_type& rhs ) const
    {
        for( std::size_t i = 0; i < ids.size(); ++i ) { if( ids[i] < rhs.ids[i] ) { return true; } }
        return false;
    }

    struct hash : public std::unary_function< key_type, std::size_t >
    {
        std::size_t operator()( const key_type& p ) const
        {
            std::size_t seed = 0;
            boost::hash_combine( seed, p.block );
            for( std::size_t i = 0; i < p.ids.size(); ++i ) { boost::hash_combine( seed, p.ids[i] ); }
            return seed;
        }
    };
};

template < typename K >
struct input
{
    key_type< K > key;
    value_type value;

    typedef boost::unordered_map< key_type< K >, std::vector< value_type >, typename key_type< K >::hash > filter_map;
};

namespace comma { namespace visiting {

template < typename T > struct traits< key_type< T > >
{
    template < typename K, typename V > static void visit( const K&, const key_type< T >& p, V& v )
    {
        v.apply( "ids", p.ids );
        v.apply( "block", p.block );
    }
    template < typename K, typename V > static void visit( const K&, key_type< T >& p, V& v )
    {
        v.apply( "ids", p.ids );
        v.apply( "block", p.block );
    }
};

template <> struct traits< value_type >
{
    template < typename K, typename V > static void visit( const K&, const value_type& p, V& v )
    {
        v.apply( "strings", p.strings );
    }
    template < typename K, typename V > static void visit( const K&, value_type& p, V& v )
    {
        v.apply( "strings", p.strings );
    }
};

template < typename T > struct traits< input< T > >
{
    template < typename K, typename V > static void visit( const K&, const input< T >& p, V& v )
    {
        v.apply( "key", p.key );
        v.apply( "value", p.value );
    }
    template < typename K, typename V > static void visit( const K&, input< T >& p, V& v )
    {
        v.apply( "key", p.key );
        v.apply( "value", p.value );
    }
};

} } // namespace comma { namespace visiting {
    

template < typename K > struct join_impl_ // quick and dirty
{
    typedef comma::csv::input_stream< input< K > > input_stream_t;
    
    typedef comma::csv::output_stream< input< K > > output_stream_t;
    
    typedef typename input< K >::filter_map filter_map_t;
    
    static filter_map_t filter_map;
    
    static typename input< K >::filter_map unmatched;
    
    static input< K > default_input;
    
    static void output_unmatched( output_stream_t& os )
    {
        if( !is_shutdown && !matched_only )
        {
            for( typename filter_map_t::const_iterator it = unmatched.begin(); it != unmatched.end(); ++it )
            {
                for( std::size_t i = 0; i < it->second.size(); ++i )
                {
                    std::cerr << "csv-advance: output_unmatched(): todo" << std::endl;
                    // todo: os.write( it->second[i].second );
                }
            }
        }
        unmatched.clear();
    }
    
    static void output_entry( const std::string& s )
    {
        if( csv.binary() ) { std::cout.write( &s[0], csv.format().size() ); std::cout.flush(); }
        else { std::cout << s << std::endl; }
    }
    
    static void output_last( const input_stream_t& is )
    {
        if( csv.binary() ) { std::cout.write( is.binary().last(), csv.format().size() ); std::cout.flush(); }
        else { std::cout << comma::join( is.ascii().last(), csv.delimiter ) << std::endl; }
    }
    
    static void read_filter_block( output_stream_t& os )
    {
        output_unmatched( os );
        static input_stream_t filter_stream( **filter_transport, csv, default_input );
        static const input< K >* last = filter_stream.read();
        if( !last ) { return; }
        block = last->key.block;
        filter_map.clear();
        comma::uint64 count = 0;
        while( last->key.block == block && !is_shutdown )
        {
            filter_map[ last->key ].push_back( last->value );
            //if( d.size() > 1 ) {}
            if( verbose ) { ++count; if( count % 10000 == 0 ) { std::cerr << "csv-update: reading block " << block << "; loaded " << count << " point[s]; hash map size: " << filter_map.size() << std::endl; } }
            //if( ( *filter_transport )->good() && !( *filter_transport )->eof() ) { break; }
            last = filter_stream.read();
            if( !last ) { break; }
        }
        unmatched = filter_map;
        if( verbose ) { std::cerr << "csv-update: read block " << block << " of " << count << " point[s]; hash map size: " << filter_map.size() << std::endl; }
    }
    
    static void update( value_type& value, const value_type& value_update, bool non_empty_only )
    {
        if( !non_empty_only ) { value = value_update; return; }
        
        // todo: handle empty value
        // todo: handle remove value
        
        for( std::size_t i = 0; i < value.strings.size(); ++i ) { if( !value_update.strings[i].empty() ) { value.strings[i] = value_update.strings[i]; } }
    }

    static int run( const comma::command_line_options& options )
    {
        std::vector< std::string > v = comma::split( csv.fields, ',' );
        for( std::size_t i = 0; i < v.size(); ++i )
        {
            if( v[i].empty() || v[i] == "block" ) { continue; }
            if( v[i] != "id" )
            { 
                if( csv.binary() ) { std::cerr << "csv-update: arbitrary fields in binary mode not supported (todo); got field: \"" << v[i] << "\"" << std::endl; return 1; }
                v[i] = "strings[" + boost::lexical_cast< std::string >( default_input.value.strings.size() ) + "]";
                default_input.value.strings.resize( default_input.value.strings.size() + 1 ); // quick and dirty
            }
            else
            {
                v[i] = "ids[" + boost::lexical_cast< std::string >( default_input.key.ids.size() ) + "]";
                default_input.key.ids.resize( default_input.key.ids.size() + 1 ); // quick and dirty
            }
        }
        if( default_input.key.ids.empty() ) { std::cerr << "csv-update: please specify at least one id field" << std::endl; return 1; }
        if( default_input.value.empty() )
        {
            
            
            
            // todo
            std::cerr << "csv-update: default fields: todo" << std::endl; return 1;
            
            
            
            
        }
        csv.fields = comma::join( v, ',' );
        input_stream_t stdin_stream( std::cin, csv, default_input );
        output_stream_t ostream( std::cout, csv, default_input );
        filter_transport.reset( new comma::io::istream( filter_name, csv.binary() ? comma::io::mode::binary : comma::io::mode::ascii ) );
        read_filter_block( ostream );
        #ifdef WIN32
        if( stdin_stream.is_binary() ) { _setmode( _fileno( stdout ), _O_BINARY ); }
        #endif
        while( !is_shutdown && ( stdin_stream.ready() || ( std::cin.good() && !std::cin.eof() ) ) )
        {
            const input< K >* p = stdin_stream.read();
            if( !p ) { break; }
            if( block != p->key.block ) { read_filter_block( ostream ); }
            typename input< K >::filter_map::const_iterator it = filter_map.find( p->key );
            if( it == filter_map.end() || it->second.empty() ) { output_last( stdin_stream ); continue; }
            input< K > current = *p;
            for( std::size_t i = 0; i < it->second.size(); ++i )
            {
                update( current.value, it->second[i], update_non_empty );
                ostream.write( current, stdin_stream ); // todo: output last only
            }
            unmatched.erase( it->first );
        }
        output_unmatched( ostream );
        return 0;
    }
};

template < typename K > typename input< K >::filter_map join_impl_< K >::unmatched;
template < typename K > typename input< K >::filter_map join_impl_< K >::filter_map;
template < typename K > input< K > join_impl_< K >::default_input;

/*

in impl/unstructured.h:

static comma::csv::format guess_format( std::istream& is, char delimiter = ',' )
{
    std::string sample;
    std::getline( is, sample, delimiter );
    
    // todo: put sample back into the stream
    
    return guess_format( sample, delimiter );
}

template < typename T, typename S >
struct keys
{
    T from;
    S to;
};

// traits for keys

boost::optional< boost::posix_time::ptime > my_lexical_cast( const std::string& s )
{
    if( s.empty() ) { return boost::optional< boost::posix_time::ptime >(); }
    return boost::posix_time::from_iso_string( s );
}

template < typename T > boost::optional< T > my_lexical_cast( const std::string& s )
{ 
    if( s.empty() ) { return boost::optional< T >(); }
    return boost::lexical_cast< T >( s );
}

template < typename From, typename To >
int run()
{
    boost::optional< From > empty_from = my_lexical_cast< From >( options.value< std::string >( "--empty" ) );
    typedef keys< From, To > keys_t;
    typedef boost::unordered_map< keys_t, std::string, hash > map_t;
    map_t map;
    comma::csv::input_stream< keys_t > istream( std::cin, csv ):
    comma::csv::output_stream< keys_t > ostream( std::cout, csv ):
    while( istream.is_ready() || std::cin.good() )
    {
        const keys_t* k = istream.read();
        if( !k ) { break; }
        if( csv.binary() )
        {
            std::string buf( csv.format().size() ); // please confirm
            ::memcpy( buf, istream.binary().last(), csv.format().size() );
            map[ *k ] = buf;
        }
        else
        {
            map[ *k ] = comma::join( istream.ascii().last(), csv.delimiter );
        }
        
        keys_t new_keys = ...; // todo: figure out the keys
        
        ostream.write( new_keys, map[ *k ] );
        
    }
    return 0;
}


template < typename T >
struct traits
{
    template < typename S >
    int run()
    {
        if( format.value( csv.field_index( "to" ) ) == "ui" ) { traits< T, comma::uint32 >::run(); }
        else if( ... )
    }
}


comma::csv::options csv( options );
comma::csv::format format = csv.binary() ? csv.format()
                                         : options.value< std::string >( "--format", "" );
if( format.string().empty() ) { format = impl::unstructured::guess_format( std::cin, csv.delimiter ); }


keys< unsigned int > k = comma::csv::ascii

if( format.value( csv.field_index( "from" ) ) == "ui" ) { traits< comma::uint32 >::run(); }
else if( ... )

cat blah.csv | csv-interval --fields=,,,from,,,,to

*/

// int main( int ac, char** av )
// {
//     try
//     {
//         comma::command_line_options options( ac, av, usage );
//         verbose = options.exists( "--verbose,-v" );
//         csv = comma::csv::options( options );
//         matched_only = options.exists( "--matched-only,--matched,-m" );
//         update_non_empty = options.exists( "--update-non-empty-fields,--update-non-empty,-u" );
//         if( csv.binary() && update_non_empty ) { std::cerr << "csv-update: --update-non-empty-fields in binary mode not supported" << std::endl; return 1; }
//         std::vector< std::string > unnamed = options.unnamed( "--matched-only,--matched,-m,--string,-s,--update-non-empty-fields,--update-non-empty,-u,--verbose,-v", "-.*" );
//         if( unnamed.empty() ) { std::cerr << "csv-update: please specify the second source" << std::endl; return 1; }
//         if( unnamed.size() > 1 ) { std::cerr << "csv-update: expected one file or stream to join, got " << comma::join( unnamed, ' ' ) << std::endl; return 1; }
//         filter_name = unnamed[0];
//         return options.exists( "--string,-s" ) ? join_impl_< std::string >::run( options ) : join_impl_< comma::int64 >::run( options );
//     }
//     catch( std::exception& ex ) { std::cerr << "csv-update: " << ex.what() << std::endl; }
//     catch( ... ) { std::cerr << "csv-update: unknown exception" << std::endl; }
//     return 1;
// }
