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

#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/scoped_ptr.hpp>
#include <comma/application/command_line_options.h>
#include <comma/application/contact_info.h>
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
    std::cerr << "    --not-equal=<value>: not equal to <value>" << std::endl;
    std::cerr << "    --greater=<value>: greater than <value>" << std::endl;
    std::cerr << "    --less=<value>: less than <value>" << std::endl;
    std::cerr << "    --from,--greater-or-equal,--ge=<value>: from <value> (inclusive, i.e. greater or equals)" << std::endl;
    std::cerr << "    --to,--less-or-equal,--le=<value>: to <value> (inclusive, i.e. less or equals)" << std::endl;
    std::cerr << std::endl;
    std::cerr << "      todo: implement a simple boolean expression grammar" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    --sorted: a hint that the key column is sorted in ascending order" << std::endl;
    std::cerr << "              todo: support descending order" << std::endl;
    std::cerr << "    --verbose,-v: more output to stderr" << std::endl;
    std::cerr << "    --or: uses 'or' expression instead of 'and' (default is 'and')" << std::endl;
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
    std::cerr << comma::contact_info << std::endl;
    std::cerr << std::endl;
    exit( 1 );
}

template < typename T >
struct constraints
{
    boost::optional< T > equals;
    boost::optional< T > not_equal;
    boost::optional< T > less;
    boost::optional< T > greater;
    boost::optional< T > from;
    boost::optional< T > to;
    bool sorted;

    constraints() : sorted( false ) {}

    constraints( const comma::command_line_options& options ) // quick and dirty
    {
        equals = options.optional< T >( "--equals" );
        not_equal = options.optional< T >( "--not-equal" );
        from = options.optional< T >( "--from,--greater-or-equal,--ge" );
        to = options.optional< T >( "--to,--less-or-equal,--le" );
        less = options.optional< T >( "--less" );
        greater = options.optional< T >( "--greater" );
        sorted = options.exists( "--sorted" );
    }

    constraints( const std::string& options, const constraints& defaults )
    {
        *this = defaults;
        comma::name_value::map m( options, ';', '=' ); // quick and dirty, since optional is not well-supported (euphymism for 'buggy') in comma::name_value::parser
        if( m.exists( "equals" ) ) { equals = m.value< T >( "equals" ); }
        if( m.exists( "not-equal" ) ) { not_equal = m.value< T >( "not-equal" ); }
        if( m.exists( "less" ) ) { less = m.value< T >( "less" ); }
        if( m.exists( "greater" ) ) { greater = m.value< T >( "greater" ); } // it was: { equal = m.value< T >( "greater" ); }
        if( m.exists( "from" ) ) { from = m.value< T >( "from" ); }
        if( m.exists( "greater-or-equal" ) ) { from = m.value< T >( "greater-or-equal" ); }
        if( m.exists( "ge" ) ) { from = m.value< T >( "ge" ); }
        if( m.exists( "to" ) ) { to = m.value< T >( "to" ); }
        if( m.exists( "less-or-equal" ) ) { to = m.value< T >( "less-or-equal" ); }
        if( m.exists( "le" ) ) { to = m.value< T >( "le" ); }
        sorted = m.exists( "sorted" );
    }

    bool is_a_match( const T& t ) const // quick and dirty, implement a proper expression parser
    {
        return    ( !equals || comma::math::equal( *equals, t ) )
               && ( !not_equal || !comma::math::equal( *not_equal, t ) )
               && ( !from || !comma::math::less( t, *from ) )
               && ( !to || !comma::math::less( *to, t ) )
               && ( !less || comma::math::less( t, *less ) )
               && ( !greater || comma::math::less( *greater, t ) );
    }

    bool done( const T& t ) const { return sorted && to && comma::math::less( *to, t ); } // quick and dirty
};

template < typename T >
struct value_t
{
    T value;
    ::constraints< T > constraints;

    bool is_a_match() const { return this->constraints.is_a_match( value ); }
    bool done() const { return this->constraints.done( value ); }
};

struct input_t
{
    std::vector< value_t< boost::posix_time::ptime > > time;
    std::vector< value_t< double > > doubles;
    std::vector< value_t< std::string > > strings;

    bool is_a_match(bool is_or) const
    {
//         std::cerr << "==> is_a_match: doubles: ";
//         for( unsigned int i = 0; i < doubles.size(); ++i ) { std::cerr << doubles[i].value << " "; }
//         std::cerr << std::endl;
        if (is_or)
        {
            for( unsigned int i = 0; i < time.size(); ++i ) { if( time[i].is_a_match() ) { return true; } }
            for( unsigned int i = 0; i < doubles.size(); ++i ) { if( doubles[i].is_a_match() ) { return true; } }
            for( unsigned int i = 0; i < strings.size(); ++i ) { if( strings[i].is_a_match() ) { return true; } }
    //        std::cerr << "==> is_a_match: done" << std::endl << std::endl;
            return false;
        }
        else
        {
            for( unsigned int i = 0; i < time.size(); ++i ) { if( !time[i].is_a_match() ) { return false; } }
            for( unsigned int i = 0; i < doubles.size(); ++i ) { if( !doubles[i].is_a_match() ) { return false; } }
            for( unsigned int i = 0; i < strings.size(); ++i ) { if( !strings[i].is_a_match() ) { return false; } }
    //        std::cerr << "==> is_a_match: done" << std::endl << std::endl;
            return true;
        }
    }

    bool done(bool is_or) const
    {
        if (is_or)
        {
            for( unsigned int i = 0; i < time.size(); ++i ) { if( !time[i].done() ) { return false; } }
            for( unsigned int i = 0; i < doubles.size(); ++i ) { if( !doubles[i].done() ) { return false; } }
            for( unsigned int i = 0; i < strings.size(); ++i ) { if( !strings[i].done() ) { return false; } }
            return true;
        }
        else
        {
            for( unsigned int i = 0; i < time.size(); ++i ) { if( time[i].done() ) { return true; } }
            for( unsigned int i = 0; i < doubles.size(); ++i ) { if( doubles[i].done() ) { return true; } }
            for( unsigned int i = 0; i < strings.size(); ++i ) { if( strings[i].done() ) { return true; } }
            return false;
        }
    }
};

namespace comma { namespace visiting {

template < typename T > struct traits< value_t< T > >
{
    template < typename K, typename V > static void visit( const K&, const value_t< T >& p, V& v )
    {
        v.apply( "value", p.value );
    }

    template < typename K, typename V > static void visit( const K&, value_t< T >& p, V& v )
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
static value_t< T > make_value( const std::string& constraints_string, const comma::command_line_options& options )
{
    static constraints< T > default_constraints( options );
    value_t< T > v;
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
                fields[i] = "t[" + boost::lexical_cast< std::string >( input.time.size() - 1 ) + "]/value";
                break;
            case comma::csv::format::fixed_string:
                input.strings.push_back( make_value< std::string >( constraints_map[ fields[i] ], options ) );
                fields[i] = "strings[" + boost::lexical_cast< std::string >( input.strings.size() - 1 ) + "]/value";
                break;
            default:
                input.doubles.push_back( make_value< double >( constraints_map[ fields[i] ], options ) );
                fields[i] = "doubles[" + boost::lexical_cast< std::string >( input.doubles.size() - 1 ) + "]/value";
                break;
        }
    }
    csv.fields = comma::join( fields, ',' );
    csv.full_xpath = true;
}

static comma::csv::format guess_format( const std::string& line )
{
    comma::csv::format format;
    std::vector< std::string > v = comma::split( line, csv.delimiter );
    for( unsigned int i = 0; i < v.size(); ++i ) // quick and dirty
    {
        try
        {
            boost::posix_time::from_iso_string( v[i] );
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
    try
    {
        comma::command_line_options options( ac, av );
        if( options.exists( "--help,-h" ) ) { usage(); }
        verbose = options.exists( "--verbose,-v" );
        bool is_or = options.exists( "--or" );
        csv = comma::csv::options( options );
        fields = comma::split( csv.fields, ',' );
        if( fields.size() == 1 && fields[0].empty() ) { fields.clear(); }
        std::vector< std::string > unnamed = options.unnamed( "--or,--sorted,--verbose,-v", "-b,--binary,-f,--fields,-d,--delimiter,--precision,--equals,--not-equal,--less-or-equal,--le,--greater-or-equal,--ge,--less,--greater,--from,--to" );
        for( unsigned int i = 0; i < unnamed.size(); constraints_map.insert( std::make_pair( comma::split( unnamed[i], ';' )[0], unnamed[i] ) ), ++i );
        if( csv.binary() )
        {
            #ifdef WIN32
            _setmode( _fileno( stdout ), _O_BINARY );
            #endif
            init_input( csv.format(), options );
            comma::csv::binary_input_stream< input_t > istream( std::cin, csv, input );
            while( istream.ready() || ( std::cin.good() && !std::cin.eof() ) )
            {
                const input_t* p = istream.read();
                if( !p || p->done(is_or) ) { break; }
                if( p->is_a_match(is_or) ) { std::cout.write( istream.last(), csv.format().size() ); std::cout.flush(); }
            }
        }
        else
        {
            boost::scoped_ptr< comma::csv::ascii_input_stream< input_t > > istream;
            while( !istream || istream->ready() || ( std::cin.good() && !std::cin.eof() ) )
            {
                if( !istream )
                {
                    std::string line;
                    while( !istream || istream->ready() || ( std::cin.good() && !std::cin.eof() ) )
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
                    if( !p || p->done(is_or) ) { break; }
                    if( p->is_a_match(is_or) ) { std::cout << line << std::endl; }

    //                 input = comma::csv::ascii< input_t >( csv.fields, csv.delimiter, csv.full_xpath, input ).get( comma::split( line, csv.delimiter ) );
    //                 if( input.done(is_or) ) { break; }
    //                 if( input.is_a_match(is_or) ) { std::cout << line << std::endl; }
                }
                else
                {
                    const input_t* p = istream->read();
                    if( !p || p->done(is_or) ) { break; }
                    if( p->is_a_match(is_or) ) { std::cout << comma::join( istream->last(), csv.delimiter ) << std::endl; }
                }
            }
        }
    }
    catch( std::exception& ex )
    {
        std::cerr << "csv-select: caught: " << ex.what() << std::endl;
    }
    catch( ... )
    {
        std::cerr << "csv-select: unknown exception" << std::endl;
    }
}
