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

/// @author matthew imhoff

#include <string.h>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <boost/unordered_map.hpp>
#include <boost/lexical_cast.hpp>
#include <comma/application/command_line_options.h>
#include <comma/application/contact_info.h>
#include <comma/base/exception.h>
#include <comma/base/types.h>
#include <comma/csv/stream.h>
#include <comma/csv/traits.h>
#include <comma/io/stream.h>
#include <comma/math/compare.h>
#include <comma/name_value/parser.h>
#include <comma/string/string.h>
#include <comma/visiting/traits.h>
#include <comma/csv/impl/unstructured.h>

static void usage( bool more )
{
    std::cerr << std::endl;
    std::cerr << "Sort a csv file using one or several keys" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Usage: cat something.csv | csv-sort [<options>]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Options:" << std::endl;
    std::cerr << "    --help,-h: help; --help --verbose: more help" << std::endl;
    std::cerr << "    --order <fields>: order in which to sort fields; default is input field order" << std::endl;
    std::cerr << "    --string,-s: keys are strings; a quick and dirty option to support strings" << std::endl;
    std::cerr << "                 default: double" << std::endl;
    std::cerr << "    --reverse,-r: sort in reverse order" << std::endl;
    std::cerr << "    --unique,-u: only outputs first line matching a given key" << std::endl;
    std::cerr << "    --verbose,-v: more output to stderr" << std::endl;
    std::cerr << "    --min: output minimum record/s based on a named field." << std::endl;
    std::cerr << "      id field(s) may be specified to select minimum record(s) in a sub-set based on the id field values, e.g. --fields=id,a,,id" << std::endl;
    std::cerr << "      id field(s) are optional, see examples" << std::endl;
    std::cerr << "      a optional 'block' field is also supported, see examples." << std::endl;
    std::cerr << "    --max: output maximum record/s a named field, see --min and examples" << std::endl;
    std::cerr << "      note that --min and --max may be used together." << std::endl;
    std::cerr << std::endl;
    std::cerr << "examples" << std::endl;
    std::cerr << "    sort by first field:" << std::endl;
    std::cerr << "        echo -e \"2\\n1\\n3\" | csv-sort --fields=a" << std::endl;
    std::cerr << "    sort by second field:" << std::endl;
    std::cerr << "        echo -e \"2,3\\n1,1\\n3,2\" | csv-sort --fields=,b" << std::endl;
    std::cerr << "    sort by second field then first field:" << std::endl;
    std::cerr << "        echo -e \"2,3\\n3,1\\n1,1\\n2,2\\n1,3\" | csv-sort --fields=a,b --order=b,a" << std::endl;
    std::cerr << "  minimum:" << std::endl;
    std::cerr << "    outputs lowest record(s) in column a." << std::endl;
    std::cerr << "        ( echo 1,a,2; echo 2,a,2; echo 3,b,3; ) | csv-sort --min --fields=,,a" << std::endl;
    std::cerr << "    outputs lowest record in field a for each ID, compare using named field a." << std::endl;
    std::cerr << "        ( echo 1,a,2; echo 2,a,2; echo 3,b,3; ) | csv-sort --min --fields=a,id" << std::endl;
    std::cerr << "    outputs lowest record with two ID fields." << std::endl;
    std::cerr << "        ( echo 1,a,1; echo 1,b,1; echo 3,b,5; echo 3,b,5; ) | csv-sort --min --fields=id,a,id" << std::endl;
    std::cerr << "    outputs lowest record(s) in column a for each block." << std::endl;
    std::cerr << "        ( echo 0,a,2; echo 0,a,2; echo 1,b,3; ) | csv-sort --min --fields=block,,a" << std::endl;
    std::cerr << "    outputs lowest record(s) in column a for each block and each unique id, for simplicity only one id field is used." << std::endl;
    std::cerr << "        ( echo 0,a,2; echo 0,a,2; echo 1,b,3; ) | csv-sort --min --fields=block,id,a" << std::endl;
    std::cerr << "  maximum:" << std::endl;
    std::cerr << "    outputs highest record(s) in column a." << std::endl;
    std::cerr << "        ( echo 1,a,2; echo 2,a,2; echo 3,b,3; ) | csv-sort --max --fields=,,a" << std::endl;
    std::cerr << "    outputs lowest record in field a for each ID in second column." << std::endl;
    std::cerr << "        ( echo 1,a,2; echo 2,a,2; echo 3,b,3; ) | csv-sort --max --fields=a,id" << std::endl;
    std::cerr << "    please see minimum for other examples" << std::endl;
    std::cerr << "  minimum and maximum:" << std::endl;
    std::cerr << "    outputs highest and lowest record(s) in column a." << std::endl;
    std::cerr << "        ( echo 1,a,2; echo 2,a,2; echo 3,b,3; echo 5,b,7; echo 3,b,9 ) | csv-sort --max --min --fields=,,a" << std::endl;
    std::cerr << "    outputs highest and lowest record(s) for each unique id." << std::endl;
    std::cerr << "        ( echo 1,a,2; echo 2,a,2; echo 3,b,3; echo 5,b,7; echo 3,b,9 ) | csv-sort --max --min --fields=,id,a" << std::endl;
    std::cerr << std::endl;
    std::cerr << comma::contact_info << std::endl;
    std::cerr << std::endl;
    if( more )
    {
        std::cerr << std::endl;
        std::cerr << "csv options:" << std::endl;
        std::cerr << comma::csv::options::usage() << std::endl;
    }
    exit( -1 );
}

static bool verbose;
static comma::csv::options stdin_csv;
static bool is_min = false;
static bool is_max = false;

struct ordering_t
{
    enum types {
        str_type,
        long_type,
        double_type,
        time_type
    };
    
    types type;
    int   index;
};

std::vector< ordering_t > ordering;

struct input_t
{
    comma::csv::impl::unstructured keys;

    bool operator==( const input_t& rhs ) const
    {
        for( std::size_t i = 0; i < ordering.size(); ++i )
        { 
            switch (ordering[i].type)
            {
                case ordering_t::str_type:
                    if (keys.strings[ordering[i].index] != rhs.keys.strings[ ordering[i].index ]) { return false; }
                    break;
                case ordering_t::long_type:
                    if (keys.longs[ordering[i].index] != rhs.keys.longs[ ordering[i].index ]) { return false; }
                    break;
                case ordering_t::double_type:
                    if (keys.doubles[ordering[i].index] != rhs.keys.doubles[ ordering[i].index ]) { return false; }
                    break;
                case ordering_t::time_type:
                    if (keys.time[ordering[i].index] != rhs.keys.time[ ordering[i].index ]) { return false; }
                    break;
            }
        }
        return true;
    }

    bool operator<( const input_t& rhs ) const
    {
        for( std::size_t i = 0; i < ordering.size(); ++i ) 
        { 
            switch (ordering[i].type)
            {
                case ordering_t::str_type:
                    if (keys.strings[ordering[i].index] < rhs.keys.strings[ ordering[i].index ]) { return true; }
                    if (keys.strings[ordering[i].index] > rhs.keys.strings[ ordering[i].index ]) { return false; }
                    break;
                case ordering_t::long_type:
                    if (keys.longs[ordering[i].index] < rhs.keys.longs[ ordering[i].index ]) { return true; }
                    if (keys.longs[ordering[i].index] > rhs.keys.longs[ ordering[i].index ]) { return false; }
                    break;
                case ordering_t::double_type:
                    if (keys.doubles[ordering[i].index] < rhs.keys.doubles[ ordering[i].index ]) { return true; }
                    if (keys.doubles[ordering[i].index] > rhs.keys.doubles[ ordering[i].index ]) { return false; }
                    break;
                case ordering_t::time_type:
                    if (keys.time[ordering[i].index] < rhs.keys.time[ ordering[i].index ]) { return true; }
                    if (keys.time[ordering[i].index] > rhs.keys.time[ ordering[i].index ]) { return false; }
                    break;
            }
        }
        return false;
    }
    
    typedef std::map< input_t, std::vector< std::string > > map;
    
};

struct input_id_t : public input_t
{
    comma::csv::impl::unstructured ids;     // IDs fields
    comma::uint32 block;
    input_id_t();
};

input_id_t::input_id_t() : block(0) {}


namespace comma { namespace visiting {

template <> struct traits< input_t >
{
    template < typename K, typename V > static void visit( const K&, const input_t& p, V& v )
    {
        v.apply( "keys", p.keys );
    }
    template < typename K, typename V > static void visit( const K&, input_t& p, V& v )
    {
        v.apply( "keys", p.keys );
    }
};

template <> struct traits< input_id_t >
{
    template < typename K, typename V > static void visit( const K& k, const input_id_t& p, V& v )
    {
        traits< input_t >::visit( k, p, v);
        v.apply( "ids", p.ids );
        v.apply( "block", p.block );
    }
    template < typename K, typename V > static void visit( const K& k, input_id_t& p, V& v )
    {
        traits< input_t >::visit( k, p, v);
        v.apply( "ids", p.ids );
        v.apply( "block", p.block );
    }
};

} } // namespace comma { namespace visiting {

template < typename It > static void output_( It it, It end )
{
    for( ; it != end; ++it )
    {
        for( std::size_t i = 0; i < it->second.size() ; ++i )
        {
            std::cout.write( &( it->second[i][0] ), stdin_csv.binary() ? stdin_csv.format().size() : it->second[i].length() );
            if( stdin_csv.flush ) { std::cout.flush(); }
        }
    }
}

typedef std::vector< std::string > records_t;
struct limit_data_t
{
    input_t keys;    /// For comparisions
    records_t records;
    void add_current_record( const comma::csv::input_stream< input_id_t >& stdin_stream )
    {
        if( stdin_stream.is_binary() )
        {
            records.push_back( std::string() );
            records.back().resize( stdin_csv.format().size() );
            ::memcpy( &records.back()[0], stdin_stream.binary().last(), stdin_csv.format().size() );
        }
        else
        {
            records.push_back( comma::join( stdin_stream.ascii().last(), stdin_csv.delimiter ) + "\n" );
        }
    }
};

/// ID key to records
typedef boost::unordered_map< comma::csv::impl::unstructured, limit_data_t, comma::csv::impl::unstructured::hash >  limit_map_t;

typedef boost::unordered_map< comma::csv::impl::unstructured, bool, comma::csv::impl::unstructured::hash >  same_map_t;
static same_map_t is_same_map;

void output_current_block( const limit_map_t& map, bool check_same_outputs=false )
{
    for( limit_map_t::const_iterator it=map.cbegin(); it!=map.cend(); ++it)
    {
        if( check_same_outputs ) {
            if( is_same_map[ it->first ] == true ) { continue; }
        }
        
        const limit_data_t& data = it->second;
        for ( std::size_t i=0; i<data.records.size(); ++i) {
            std::cout.write( &( data.records[i][0] ), stdin_csv.binary() ? stdin_csv.format().size() : data.records[i].length() );
        }
        if( stdin_csv.flush ) { std::cout.flush(); }
    }
}

struct min_max_t
{
    /// Save data into the records_t collection
    static void save( const comma::csv::options& stdin_csv, const comma::csv::input_stream< input_id_t >& stdin_stream, records_t& d  )
    {
        if( stdin_stream.is_binary() )
        {
            d.push_back( std::string() );
            d.back().resize( stdin_csv.format().size() );
            ::memcpy( &d.back()[0], stdin_stream.binary().last(), stdin_csv.format().size() );
        }
        else
        {
            d.push_back( comma::join( stdin_stream.ascii().last(), stdin_csv.delimiter ) + "\n" );
        }
    }
};


int min_max_select( const comma::command_line_options& options )
{
    input_id_t default_input;
    std::vector< std::string > v = comma::split( stdin_csv.fields, ',' );
    std::vector< std::string > w (v.size());
    
    std::string first_line;
    comma::csv::format f;
    if( stdin_csv.binary() ) { f = stdin_csv.format(); }
    else if( options.exists( "--format" ) ) { f = comma::csv::format( options.value< std::string >( "--format" ) ); }
    else
    {
        while( std::cin.good() && first_line.empty() ) { std::getline( std::cin, first_line ); }
        if( first_line.empty() ) { return 0; }
        f = comma::csv::impl::unstructured::guess_format( first_line, stdin_csv.delimiter );
    }
    
    comma::uint32 keys_size = 0;
    for( std::size_t k=0; k<v.size(); ++k )
    {
        const std::string& field = v[k];
        if( field.empty() ) { }
        else if( field == "id" ) { w[k] = "ids/" + default_input.ids.append( f.offset( k ).type ); } // std::cerr  << "appended " << w[k] << std::endl; }
        else if( field == "block" ) { w[k] = "block"; } // std::cerr  << "appended " << w[k] << std::endl; }
        else 
        {
            ordering.push_back( ordering_t() );
            std::string type = default_input.keys.append( f.offset( k ).type ); 
            if ( type[0] == 's' ) {      ordering.back().type = ordering_t::str_type; }
            else if ( type[0] == 'l' ) { ordering.back().type = ordering_t::long_type; }
            else if ( type[0] == 'd' ) { ordering.back().type = ordering_t::double_type; }
            else if ( type[0] == 't' ) { ordering.back().type = ordering_t::time_type; }
            ordering.back().index = keys_size;
            w[k] = "keys/" + type; 
            ++keys_size; 
        }
    }
    
    is_min = options.exists( "--min" );
    is_max = options.exists( "--max" );
    if( keys_size != 1 ) { std::cerr << "csv-sort: error, please specify exactly one field for --min/--max operation." << std::endl; return 1; }
    
    if ( verbose ) { std::cerr << "csv-sort: minimum mode: " << ( is_min ) << ", maximum mode: " << is_max  << std::endl; }
    stdin_csv.fields = comma::join( w, ',' );
    if ( verbose ) { std::cerr << "csv-sort: fields: " << stdin_csv.fields << std::endl; }
    if( verbose ) { std::cerr << "csv-sort: guessed format: " << f.string() << std::endl; }
    comma::csv::input_stream< input_id_t > stdin_stream( std::cin, stdin_csv, default_input );
    #ifdef WIN32
    if( stdin_stream.is_binary() ) { _setmode( _fileno( stdout ), _O_BINARY ); }
    #endif
    
    records_t min;
    records_t max;
    input_id_t prev_id;
    comma::uint32 block = 0;
    
    limit_map_t min_map;
    limit_map_t max_map;
    
    bool first = true;
    if (!first_line.empty()) 
    { 
        input_id_t input =  comma::csv::ascii< input_id_t >(stdin_csv,default_input).get(first_line);
        limit_data_t& data = min_map[input.ids];
        data.keys = input;
        data.records.push_back( first_line + "\n");
        
        max_map[input.ids] = data;
        is_same_map[input.ids] = true;
        first = false;
    }
    while( stdin_stream.ready() || ( std::cin.good() && !std::cin.eof() ) )
    {
        const input_id_t* p = stdin_stream.read();
        if( !p ) { break; }
//         std::cerr  << "p: " << comma::join( stdin_stream.ascii().last(), stdin_csv.delimiter ) << " - " << p->keys.longs[0] << std::endl;
        
        if( first )
        {
            limit_data_t& data = min_map[p->ids];
            data.keys = *p;
            data.add_current_record( stdin_stream );
            
            max_map[p->ids] = data;
            is_same_map[p->ids] = true;
            
            first = false;
        }
        else if( p->block != block )
        {
            // Dump and clear previous
            output_current_block( min_map );
            output_current_block( max_map, is_min && is_max );
            min_map.clear();
            max_map.clear();
            
            limit_data_t& data = min_map[p->ids];
            data.keys = *p;
            data.add_current_record( stdin_stream );
            
            max_map[p->ids] = data;
            is_same_map[p->ids] = true;
            
            block = p->block;
        }
        else    /// No ID or same ID as prev record, compare to append or replace
        {
            if( is_min )
            {
                limit_map_t::iterator iter = min_map.find( p->ids );
                if( iter == min_map.end() )
                {
//                     std::cerr  << "not found ids: " << p->ids.longs[0] << std::endl;
                    limit_data_t& data = min_map[p->ids];
                    data.keys = *p;
                    data.add_current_record( stdin_stream );
                    is_same_map[p->ids] = true;
                }
                else
                {
//                     std::cerr  << "found ids: " << p->ids.longs[0] << std::endl;
                    limit_data_t& data = iter->second;
//                     std::cerr  << "found ids after: " << std::endl;
                    if( data.keys == *p ) { data.add_current_record( stdin_stream ); } // std::cerr  << "equals " << std::endl; } // Else If equals then append
                    else if( *p < data.keys )
                    {
//                         std::cerr  << "new min: " << p->ids.longs[0] << std::endl;
                        data.keys = *p;
                        data.records.clear();
                        data.add_current_record( stdin_stream );
                        is_same_map[p->ids] = false;
                    }
//                     else { std::cerr  << "else: " << std::endl; }
                }
            }
            if( is_max )
            {
                limit_map_t::iterator iter = max_map.find( p->ids );
                if( iter == max_map.end() )
                {
//                     std::cerr  << "not found ids: " << p->ids.strings[0] << std::endl;
                    limit_data_t& data = max_map[p->ids];
                    data.keys = *p;
                    data.add_current_record( stdin_stream );
                    is_same_map[p->ids] = true;
                }
                else
                {
//                     std::cerr  << "found ids: " << p->ids.strings[0] << std::endl;
                    limit_data_t& data = iter->second;
                    if( *p < data.keys ) {}
                    else if( data.keys == *p ) { data.add_current_record( stdin_stream ); } //  std::cerr  << "equals " << std::endl; } // Else If equals then append
                    else
                    {
//                         std::cerr  << "new max: " << p->ids.strings[0] << " " << p->keys.longs[0] << " " << data.keys.keys.longs[0] << std::endl;
                        data.keys = *p;
                        data.records.clear();
                        data.add_current_record( stdin_stream );
                        is_same_map[p->ids] = false;
                    }
                }
            }
        }
        
    }
//     std::cerr  << "last dump min: " << min_map.size() << std::endl;
//     std::cerr  << "last dump max: " << max_map.size() << std::endl;
    
    if( is_min ) { output_current_block( min_map ); }
    if( is_max ) { output_current_block( max_map, is_min && is_max ); }
    
    return 0;
}
int sort( const comma::command_line_options& options )
{
    input_t::map sorted_map;
    input_t default_input;
    std::vector< std::string > v = comma::split( stdin_csv.fields, ',' );
    std::vector< std::string > order = options.exists("--order") ? comma::split( options.value< std::string >( "--order" ), ',' ) : v;
    std::vector< std::string > w (v.size());
    bool unique = options.exists("--unique,-u");
    
    std::string first_line;
    comma::csv::format f;
    if( stdin_csv.binary() ) { f = stdin_csv.format(); }
    else if( options.exists( "--format" ) ) { f = comma::csv::format( options.value< std::string >( "--format" ) ); }
    else
    {
        while( std::cin.good() && first_line.empty() ) { std::getline( std::cin, first_line ); }
        if( first_line.empty() ) { return 0; }
        f = comma::csv::impl::unstructured::guess_format( first_line, stdin_csv.delimiter );
        if( verbose ) { std::cerr << "csv-sort: guessed format: " << f.string() << std::endl; }
    }
    for( std::size_t i = 0; i < order.size(); ++i ) // quick and dirty, wasteful, but who cares
    {
        if (order[i].empty()) continue;
        for( std::size_t k = 0; k < v.size(); ++k )
        {
            if( v[k].empty() || v[k] != order[i] ) 
            { 
                if ( k + 1 == v.size()) 
                { 
                    std::cerr << "csv-sort: order field name \"" << order[i] << "\" not found in input fields \"" << stdin_csv.fields << "\"" << std::endl;
                    return 1;
                }
                continue; 
            }
            std::string type = default_input.keys.append( f.offset( k ).type );
            w[k] = "keys/" + type;
            
            ordering_t o;
            if ( type[0] == 's' ) { o.type = ordering_t::str_type; o.index = default_input.keys.strings.size() - 1; }
            else if ( type[0] == 'l' ) { o.type = ordering_t::long_type; o.index = default_input.keys.longs.size() - 1; }
            else if ( type[0] == 'd' ) { o.type = ordering_t::double_type; o.index = default_input.keys.doubles.size() - 1; }
            else if ( type[0] == 't' ) { o.type = ordering_t::time_type; o.index = default_input.keys.time.size() - 1; }
            else { std::cerr << "csv-sort: cannot sort on field " << v[k] << " of type \"" << type << "\"" << std::endl; return 1; }
            
            ordering.push_back(o);
            
            break;
        }
    }
    stdin_csv.fields = comma::join( w, ',' );
    if ( verbose ) { std::cerr << "csv-sort: fields: " << stdin_csv.fields << std::endl; }
    comma::csv::input_stream< input_t > stdin_stream( std::cin, stdin_csv, default_input );
    #ifdef WIN32
    if( stdin_stream.is_binary() ) { _setmode( _fileno( stdout ), _O_BINARY ); }
    #endif
    
    if (!first_line.empty()) 
    { 
        input_t::map::mapped_type& d = sorted_map[ comma::csv::ascii< input_t >(stdin_csv,default_input).get(first_line) ];
        d.push_back( first_line + "\n" );
    }
    
    while( stdin_stream.ready() || ( std::cin.good() && !std::cin.eof() ) )
    {
        const input_t* p = stdin_stream.read();
        if( !p ) { break; }
        if( stdin_stream.is_binary() )
        {
            input_t::map::mapped_type& d = sorted_map[ *p ];
            if (unique && !d.empty()) continue;
            d.push_back( std::string() );
            d.back().resize( stdin_csv.format().size() );
            ::memcpy( &d.back()[0], stdin_stream.binary().last(), stdin_csv.format().size() );
        }
        else
        {
            input_t::map::mapped_type& d = sorted_map[ *p ];
            if (unique && !d.empty()) continue;
            d.push_back( comma::join( stdin_stream.ascii().last(), stdin_csv.delimiter ) + "\n" );
        }
    }
    
    if( options.exists( "--reverse,-r" ) ) { output_( sorted_map.rbegin(), sorted_map.rend() ); }
    else { output_( sorted_map.begin(), sorted_map.end() ); }
    
    return 0;
}

int main( int ac, char** av )
{
    comma::command_line_options options( ac, av, usage );
    try
    {
        verbose = options.exists( "--verbose,-v" );
        stdin_csv = comma::csv::options( options );
        stdin_csv.full_xpath = true;
        if( options.exists("--min,--max") ) { return min_max_select( options ); } else { return sort( options ); }
    }
    catch( std::exception& ex )
    {
        std::cerr << "csv-sort: " << ex.what() << std::endl << comma::join(options.argv(), ' ') << std::endl;
    }
    catch( ... )
    {
        std::cerr << "csv-sort: unknown exception" << std::endl;
    }
    return 1;
}
