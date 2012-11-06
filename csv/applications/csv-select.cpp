// This file is part of comma, a generic and flexible library 
// for robotics research.
//
// Copyright (C) 2011 The University of Sydney
//
// comma is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// comma is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License 
// for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with comma. If not, see <http://www.gnu.org/licenses/>.

/// @author vsevolod vlaskine

#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/scoped_ptr.hpp>
#include <comma/application/command_line_options.h>
#include <comma/application/signal_flag.h>
#include <comma/csv/stream.h>
#include <comma/math/compare.h>
#include <comma/name_value/parser.h>
#include <comma/string/string.h>
#include <comma/visiting/traits.h>

void usage()
{
    std::cerr << std::endl;
    std::cerr << "find in a file or stream by constraints on a given key" << std::endl;
    std::cerr << std::endl;
    std::cerr << "usage: cat a.csv | csv-select <options> [<key properties>]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options" << std::endl;
    std::cerr << "    --equals=<value>: equals to <value>" << std::endl;
    std::cerr << "    --from=<value>: from <value> (inclusive, i.e. greater or equals)" << std::endl;
    std::cerr << "    --to=<value>: to <value> (inclusive, i.e. less or equals)" << std::endl;
    std::cerr << "      todo: implement a simple boolean expression grammar" << std::endl;
    std::cerr << "    --sorted: a hint that the key column is sorted in ascending order" << std::endl;
    std::cerr << "              todo: support descending order" << std::endl;
    std::cerr << "    --verbose,-v: more output to stderr" << std::endl;
    std::cerr << std::endl;
    std::cerr << "fields: any non-empty fields will be treated as keys" << std::endl;
    std::cerr << std::endl;
    std::cerr << "csv options" << std::endl;
    std::cerr << comma::csv::options::usage() << std::endl;
    std::cerr << std::endl;
    std::cerr << "examples" << std::endl;
    std::cerr << "    cat a.csv | csv-select --fields=,,t --from=20120101T000000" << std::endl;
    std::cerr << "    cat a.csv | csv-select --fields=,,t --from=20120101T000000 --to=20120101T000010 --sorted" << std::endl;
    std::cerr << "    cat xyz.csv | csv-select --fields=x,y,z \"x;from=1;to=2\" \"y;from=-1;to=1.1\" \"z;from=5;to=5.5\"" << std::endl;
    std::cerr << "    cat a.csv | csv-select --fields=t,scalar \"t;from=20120101T000000;sorted\" \"scalar;from=-10;to=20.5\"" << std::endl;
    std::cerr << std::endl;
    exit( 1 );
}

template < typename T >
struct constraints
{
    boost::optional< T > equals;
    boost::optional< T > from;
    boost::optional< T > to;
    bool sorted;
    
    constraints() : sorted( false ) {}
    
    constraints( const comma::command_line_options& options ) // quick and dirty
    {
        equals = options.optional< T >( "--equals" );
        from = options.optional< T >( "--from" );
        to = options.optional< T >( "--to" );
        sorted = options.exists( "--sorted" );
    }
    
    constraints( const std::string& options, const constraints& defaults )
    {
        *this = defaults;
        comma::name_value::map m( options, ';', '=' ); // quick and dirty, since optional is not well-supported in comma::name_value::parser
        if( m.exists( "equals" ) ) { equals = m.value< T >( "equals" ); }
        if( m.exists( "from" ) ) { from = m.value< T >( "from" ); }
        if( m.exists( "to" ) ) { to = m.value< T >( "to" ); }
        sorted = m.exists( "sorted" );
    }
    
    bool is_a_match( const T& t ) const
    {
        if( equals ) { std::cerr << "==> is_a_match: t: " << t << " equals: " << *equals << std::endl; }
        if( from ) { std::cerr << "==> is_a_match: t: " << t << " from: " << *from << std::endl; }
        if( to ) { std::cerr << "==> is_a_match: t: " << t << " to: " << *to << std::endl; }
        
        return    ( !equals || comma::math::equal( *equals, t ) )
               && ( !from || !comma::math::less( t, *from ) )
               && ( !to || !comma::math::less( *to, t ) );
    }
    
    bool done( const T& t ) const { return sorted && to && comma::math::less( *to, t ); } // quick and dirty
};

template < typename T >
struct value
{
    T value;
    ::constraints< T > constraints;
    
    bool is_a_match() const { return this->constraints.is_a_match( value ); }
    bool done() const { return this->constraints.done( value ); }
};

struct input_t
{
    std::vector< value< boost::posix_time::ptime > > time;
    std::vector< value< double > > doubles;
    std::vector< value< std::string > > strings;
    
    bool is_a_match() const
    {
        std::cerr << "==> is_a_match: doubles.size(): " << doubles.size() << std::endl;
        for( unsigned int i = 0; i < time.size(); ++i ) { if( !time[i].is_a_match() ) { return false; } }
        for( unsigned int i = 0; i < doubles.size(); ++i ) { if( !doubles[i].is_a_match() ) { return false; } }
        for( unsigned int i = 0; i < strings.size(); ++i ) { if( !strings[i].is_a_match() ) { return false; } }
        std::cerr << "==> is_a_match: done" << std::endl;
        return true;
    }
    
    bool done() const
    {
        for( unsigned int i = 0; i < time.size(); ++i ) { if( time[i].done() ) { return true; } }
        for( unsigned int i = 0; i < doubles.size(); ++i ) { if( doubles[i].done() ) { return true; } }
        for( unsigned int i = 0; i < strings.size(); ++i ) { if( strings[i].done() ) { return true; } }
        return false;
    }
};

namespace comma { namespace visiting {

template < typename T > struct traits< value< T > >
{
    template < typename K, typename V > static void visit( const K&, const value< T >& p, V& v )
    { 
        v.apply( "value", p.value );
    }
    
    template < typename K, typename V > static void visit( const K&, value< T >& p, V& v )
    { 
        v.apply( "value", p.value );
    }
};
    
template <> struct traits< input_t >
{
    template < typename K, typename V > static void visit( const K&, const input_t& p, V& v )
    { 
        v.apply( "t", p.time );
        v.apply( "doubles", p.doubles );
        v.apply( "strings", p.strings );
    }
    
    template < typename K, typename V > static void visit( const K&, input_t& p, V& v )
    { 
        v.apply( "t", p.time );
        v.apply( "doubles", p.doubles );
        v.apply( "strings", p.strings );
    }
};    

} } // namespace comma { namespace visiting {

template < typename T >
static value< T > make_value( const std::string& constraints_string, const comma::command_line_options& options )
{
    static constraints< T > default_constraints( options );
    value< T > v;
    v.constraints = constraints< T >( constraints_string, default_constraints );
    return v;
}

static bool verbose;
static comma::csv::options csv;
static input_t input;
static std::vector< std::string > fields;
static std::map< std::string, std::string > constraints_map;

static void init_input( const comma::csv::format& format, const comma::command_line_options& options )
{
    if( fields.empty() ) { for( unsigned int i = 0; i < format.count(); ++i ) { fields.push_back( "v" ); } }
    for( unsigned int i = 0; i < fields.size(); ++i )
    {
        if( comma::strip( fields[i], ' ' ).empty() ) { continue; }
        switch( format.offset( i ).type )
        {
            case comma::csv::format::time:
            case comma::csv::format::long_time:
                input.time.push_back( make_value< boost::posix_time::ptime >( constraints_map[ fields[i] ], options ) );
                fields[i] = "t[" + boost::lexical_cast< std::string >( input.time.size() - 1 ) + "]";
                break;
            case comma::csv::format::fixed_string:
                input.strings.push_back( make_value< std::string >( constraints_map[ fields[i] ], options ) );
                fields[i] = "strings[" + boost::lexical_cast< std::string >( input.strings.size() - 1 ) + "]";
                break;
            default:
                input.doubles.push_back( make_value< double >( constraints_map[ fields[i] ], options ) );
                fields[i] = "doubles[" + boost::lexical_cast< std::string >( input.doubles.size() - 1 ) + "]";
                break;
        }
    }
    csv.fields = comma::join( fields, ',' );    
}

static comma::csv::format guess_format( const std::string& line )
{
    comma::csv::format format;
    std::vector< std::string > v = comma::split( line, csv.delimiter );
    for( unsigned int i = 0; i < v.size(); ++i ) // quick and dirty
    {
        try
        {
            boost::lexical_cast< boost::posix_time::ptime >( v[i] );
            format += "t";
        }
        catch( ... )
        {
            try
            {
                boost::lexical_cast< double >( v[i] );
                format += "d";
            }
            catch( ... )
            {
                format += "s[1024]"; // quick and dirty
            }
        }
    }
    if( verbose ) { std::cerr << "csv-select: guessed format: \"" << format.string() << "\"" << std::endl; }
    return format;
}

int main( int ac, char** av )
{
    comma::command_line_options options( ac, av );
    if( options.exists( "--help,-h" ) ) { usage(); }
    verbose = options.exists( "--verbose,-v" );
    csv = comma::csv::options( options );
    csv.full_xpath = false;
    fields = comma::split( csv.fields, ',' );
    if( fields.size() == 1 && fields[0].empty() ) { fields.clear(); }
    std::vector< std::string > unnamed = options.unnamed( "--sorted,--verbose,-v", "-b,--binary,-f,--fields,-d,--delimiter,--precision,--equals,--from,--to" );
    for( unsigned int i = 0; i < unnamed.size(); constraints_map.insert( std::make_pair( comma::split( unnamed[i], ';' )[0], unnamed[i] ) ), ++i );
    comma::signal_flag is_shutdown;
    if( csv.binary() )
    {
        #ifdef WIN32
        _setmode( _fileno( stdout ), _O_BINARY );
        #endif
        init_input( csv.format(), options );
        csv.fields = comma::join( fields, ',' );
        comma::csv::binary_input_stream< input_t > istream( std::cin, csv, input );
        while( !is_shutdown && std::cin.good() && !std::cin.eof() )
        {
            const input_t* p = istream.read();
            if( !p || p->done() ) { break; }
            if( p->is_a_match() ) { std::cout.write( istream.last(), csv.format().size() ); std::cout.flush(); }
        }
    }
    else
    {
        csv.fields = comma::join( fields, ',' );
        boost::scoped_ptr< comma::csv::ascii_input_stream< input_t > > istream;
        while( !is_shutdown && std::cin.good() && !std::cin.eof() )
        {
            if( !istream )
            {
                std::string line;
                while( !is_shutdown && std::cin.good() && !std::cin.eof() )
                {
                    std::getline( std::cin, line );
                    line = comma::strip( line, '\r' );
                    if( !line.empty() ) { break; }
                }
                if( line.empty() ) { break; }
                comma::csv::format format = guess_format( line );
                init_input( format, options );
                istream.reset( new comma::csv::ascii_input_stream< input_t >( std::cin, csv, input ) );
                
                // todo: quick and dirty: no time to debug why the commented section does not work (but that's the right way)
                std::istringstream iss( line );
                comma::csv::ascii_input_stream< input_t > isstream( iss, csv, input );
                const input_t* p = isstream.read();
                if( p->done() ) { break; }
                if( p->is_a_match() ) { std::cout << line << std::endl; }

//                 input = comma::csv::ascii< input_t >( csv.fields, csv.delimiter, csv.full_xpath, input ).get( comma::split( line, csv.delimiter ) );
//                 if( input.done() ) { break; }
//                 if( input.is_a_match() ) { std::cout << line << std::endl; }
            }
            else
            {
                const input_t* p = istream->read();
                if( !p || p->done() ) { break; }
                if( p->is_a_match() ) { std::cout << comma::join( istream->last(), csv.delimiter ) << std::endl; }
            }
        }
    }
}
