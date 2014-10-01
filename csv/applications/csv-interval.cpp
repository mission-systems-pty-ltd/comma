// This file is part of comma, a generic and flexible library
// Copyright (c) 2014 The University of Sydney
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


/// @author Vinny Do

#include <iostream>
#include <set>
#include <string>
#include <boost/icl/interval.hpp>
#include <boost/icl/interval_map.hpp>
#include "comma/application/command_line_options.h"
#include "comma/base/exception.h"
#include "comma/csv/stream.h"
#include "comma/visiting/traits.h"
#include "comma/csv/impl/unstructured.h"

static const std::string app_name = "csv-interval";

static bool verbose;
static bool debug;
static std::string first_line;

template < typename T > struct limits
{
    static T max() { return std::numeric_limits< T >::max(); }
    static T lowest() { return std::numeric_limits< T >::min(); }
};

template <> struct limits< float >
{
    static float max() { return std::numeric_limits< float >::max(); }
    static float lowest() { return -max(); }
};

template <> struct limits< double >
{
    static double max() { return std::numeric_limits< double >::max(); }
    static double lowest() { return -max(); }
};

template <> struct limits< long double >
{
    static long double max() { return std::numeric_limits< long double >::max(); }
    static long double lowest() { return -max(); }
};

template <> struct limits< boost::posix_time::ptime >
{
    // static boost::posix_time::ptime max() { return boost::posix_time::pos_infin; }
    // static boost::posix_time::ptime lowest() { return boost::posix_time::neg_infin; }
    static boost::posix_time::ptime max() { return boost::posix_time::not_a_date_time; }
    static boost::posix_time::ptime lowest() { return boost::posix_time::not_a_date_time; }
};

static void usage( bool verbose = false )
{
    std::cerr << "takes csv intervals and separates them at points of overlap if any" << std::endl;
    std::cerr << std::endl;
    std::cerr << "usage: cat intervals.csv | " << app_name << " [OPTIONS...]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options" << std::endl;
    std::cerr << "    --help,-h: show help; --help --verbose for more help" << std::endl;
    std::cerr << "    --verbose,-v: more info" << std::endl;
    std::cerr << "    --debug: print debug" << std::endl;
    std::cerr << "    --input-fields: print input fields and exit" << std::endl;
    // std::cerr << "    --input-format: print input format and exit" << std::endl;
    std::cerr << "    --output-fields: print output fields and exit" << std::endl;
    // std::cerr << "    --output-format: print output format and exit" << std::endl;
    std::cerr << "    --empty: empty value used to signify unbounded intervals" << std::endl;
    std::cerr << "             default for time is \"not-a-date-time\"" << std::endl;
    std::cerr << "    --format: input format (ascii only), also affects the --limits option; if not given the format is guessed" << std::endl;
    std::cerr << "    --intervals-only: only output the intervals, ignore payload if any" << std::endl;
    std::cerr << "    --limits,-l: replace empty bounds with type limits" << std::endl;
    std::cerr << "                  b  : " << (int)limits< char >::lowest() << " " << (int)limits< char >::max() << std::endl;
    std::cerr << "                  ub : " << (int)limits< unsigned char >::lowest() << " " << (int)limits< unsigned char >::max() << std::endl;
    std::cerr << "                  w  : " << limits< comma::int16 >::lowest() << " " << limits< comma::int16 >::max() << std::endl;
    std::cerr << "                  uw : " << limits< comma::uint16 >::lowest() << " " << limits< comma::uint16 >::max() << std::endl;
    std::cerr << "                  i  : " << limits< comma::int32 >::lowest() << " " << limits< comma::int32 >::max() << std::endl;
    std::cerr << "                  ui : " << limits< comma::uint32 >::lowest() << " " << limits< comma::uint32 >::max() << std::endl;
    std::cerr << "                  l  : " << limits< comma::int64 >::lowest() << " " << limits< comma::int64 >::max() << std::endl;
    std::cerr << "                  ul : " << limits< comma::uint64 >::lowest() << " " << limits< comma::uint64 >::max() << std::endl;
    std::cerr << "                  c  : " << (int)limits< char >::lowest() << " " << (int)limits< char >::max() << std::endl;
    std::cerr << "                  f  : " << limits< float >::lowest() << " " << limits< float >::max() << std::endl;
    std::cerr << "                  d  : " << limits< double >::lowest() << " " << limits< double >::max() << std::endl;
    std::cerr << "                  s  : \"" << limits< std::string >::lowest() << "\" \"" << limits< std::string >::max() << "\"" << std::endl;
    std::cerr << "                  t  : " << limits< boost::posix_time::ptime >::lowest() << " " << limits< boost::posix_time::ptime >::max() << std::endl;
    std::cerr << "                  lt : " << limits< boost::posix_time::ptime >::lowest() << " " << limits< boost::posix_time::ptime >::max() << std::endl;
    std::cerr << std::endl;
    std::cerr << "ascii notes" << std::endl;
    std::cerr << "    unbounded intervals may be indicated by no value (e.g. ,3 \u2261 -\u221e,3), both sides unbounded is also supported" << std::endl;
    std::cerr << "    if --empty is not given, numeric bounds with no value are output as their type limits" << std::endl;
    std::cerr << "    todo: add csv quote option so that bounds with no value may be output unquoted" << std::endl;
    std::cerr << std::endl;
    std::cerr << "for examples see verbose help: " << app_name << " --help --verbose" << std::endl;
    std::cerr << std::endl;
    if( verbose )
    {
        std::cerr << "csv options" << std::endl;
        std::cerr << comma::csv::options::usage() << std::endl;
        std::cerr << std::endl;
        std::cerr << "examples" << std::endl;
        std::cerr << std::endl;
        std::cerr << "    basic example" << std::endl;
        std::cerr << std::endl;
        std::cerr << "        A: [1                5]"      << std::endl;
        std::cerr << "        B:      [2      4]"           << std::endl;
        std::cerr << "        C:           [3           6]" << std::endl;
        std::cerr << std::endl;
        std::cerr << "        echo -e '1,5,A\\n2,4,B\\n3,6,C' | " << app_name << std::endl;
        std::cerr << std::endl;
        std::cerr << "        A: [1 2][2 3][3 4][4 5]"      << std::endl;
        std::cerr << "        B:      [2 3][3 4]"           << std::endl;
        std::cerr << "        C:           [3 4][4 5][5 6]" << std::endl;
        std::cerr << std::endl;
        std::cerr << "    unbounded example" << std::endl;
        std::cerr << std::endl;
        std::cerr << "        A: [-\u221e           4]"                              << std::endl;
        std::cerr << "        B:       [2      4]"                              << std::endl;
        std::cerr << "        C:            [3      6]"                         << std::endl;
        std::cerr << "        D:            [3           8]"                    << std::endl;
        std::cerr << "        Z: [-\u221e                          +\u221e]"    << std::endl;
        std::cerr << std::endl;
        std::cerr << "        echo -e ',4,A\\n2,4,B\\n3,6,C\\n3,8,D\\n,,Z' | " << app_name << " --format 2i" << std::endl;
        std::cerr << std::endl;
        std::cerr << "        A: [-\u221e 2][2 3][3 4]"                         << std::endl;
        std::cerr << "        B:       [2 3][3 4]"                              << std::endl;
        std::cerr << "        C:            [3 4][4 6]"                         << std::endl;
        std::cerr << "        D:            [3 4][4 6][6 8]"                    << std::endl;
        std::cerr << "        Z: [-\u221e 2][2 3][3 4][4 6][6 8][8 +\u221e]"    << std::endl;
        std::cerr << std::endl;
        std::cerr << "    unbounded time example" << std::endl;
        std::cerr << std::endl;
        std::cerr << "        A: [-\u221e                                  20140916T030000]" << std::endl;
        std::cerr << "        B:                     [20140916T010000                                  20140916T190000]" << std::endl;
        std::cerr << "        C:                                                                                       [20140916T190000 +\u221e]" << std::endl;
        std::cerr << "        Z: [-\u221e                                                                                                    +\u221e]" << std::endl;
        std::cerr << std::endl;
        std::cerr << "        echo -e ',20140916T030000.000000,A\\n20140916T010000.000000,20140916T190000.000000,B\\n20140916T190000.000000,,C\\n,,Z' | " << app_name << " --format 2t" << std::endl;
        std::cerr << std::endl;
        std::cerr << "        A: [-\u221e 20140916T010000][20140916T010000 20140916T030000]" << std::endl;
        std::cerr << "        B:                     [20140916T010000 20140916T030000][20140916T030000 20140916T190000]" << std::endl;
        std::cerr << "        C:                                                                                       [20140916T190000 +\u221e]" << std::endl;
        std::cerr << "        Z: [-\u221e 20140916T010000][20140916T010000 20140916T030000][20140916T030000 20140916T190000][20140916T190000 +\u221e]" << std::endl;
        std::cerr << std::endl;
    }
    exit( 1 );
}

enum { LOWER, UPPER };

/// wrapper class which allows for a bound which:
///     * may be unbounded
///     * has indicator of side (lower or upper)
///
///  a lower bound with no value implies -infinity
/// an upper bound with no value implies +infinity
template < typename T >
struct bound_t
{
    unsigned int side;
    boost::optional< T > value;
    bound_t() {}
    bound_t( const unsigned int side ) : side( side ) {}
    bound_t( const unsigned int side, const boost::optional< T >& value ) : side( side ), value( value ) {}

    bool operator<( const bound_t& rhs ) const
    {
        if( value && rhs.value ) { return *value < *rhs.value; }
        if( !value ) { return side == LOWER; }
        return rhs.side == UPPER;
    }

    friend std::ostream& operator<<( std::ostream& os, const bound_t& b ) { return b.value ? os << *b.value : os << ( b.side == LOWER ? "-" : "+" ) << "\u221e"; }
};

template < typename T > struct bound_traits { typedef T type; };
template <> struct bound_traits< char > { typedef double type; };
template <> struct bound_traits< unsigned char > { typedef double type; };
template <> struct bound_traits< comma::int16 > { typedef double type; };
template <> struct bound_traits< comma::uint16 > { typedef double type; };
template <> struct bound_traits< comma::int32 > { typedef double type; };
template <> struct bound_traits< comma::uint32 > { typedef double type; };
template <> struct bound_traits< comma::int64 > { typedef double type; };
template <> struct bound_traits< comma::uint64 > { typedef double type; };
template <> struct bound_traits< float > { typedef double type; };

template < typename T > struct traits
{
    static T default_() { return T(); }
    static boost::optional< T > cast( const boost::optional< std::string >& s ) { if( !s ) { return boost::none; } return boost::lexical_cast< T >( *s ); }
};

template <> struct traits< boost::posix_time::ptime >
{
    static boost::posix_time::ptime default_() { return boost::posix_time::not_a_date_time; }
    static boost::optional< boost::posix_time::ptime > cast( const boost::optional< std::string >& s )
    {
        if( !s ) { return default_(); }
        try { return boost::posix_time::from_iso_string( *s ); } catch( ... ) { }
        return default_();
    }
};

template < typename T > struct from_t { T value; from_t() : value( traits< T >::default_() ) { } };
template < typename T > struct to_t { T value; to_t() : value( traits< T >::default_() ) { } };

template < typename From, typename To = From >
struct interval_t
{
    from_t< From > from;
    to_t< To > to;
};

namespace comma { namespace visiting {

template < typename T > struct traits< from_t< T > >
{
    template < typename K, typename V > static void visit( const K&, from_t< T >& p, V& v ) { v.apply( "from", p.value ); }
    template < typename K, typename V > static void visit( const K&, const from_t< T >& p, V& v ) { v.apply( "from", p.value ); }
};

template < typename T > struct traits< to_t< T > >
{
    template < typename K, typename V > static void visit( const K&, to_t< T >& p, V& v ) { v.apply( "to", p.value ); }
    template < typename K, typename V > static void visit( const K&, const to_t< T >& p, V& v ) { v.apply( "to", p.value ); }
};

template < typename From, typename To > struct traits< interval_t< From, To > >
{
    template < typename K, typename V > static void visit( const K&, interval_t< From, To >& p, V& v ) { v.apply( "", p.from ); v.apply( "", p.to ); }
    template < typename K, typename V > static void visit( const K&, const interval_t< From, To >& p, V& v ) { v.apply( "", p.from ); v.apply( "", p.to ); }
};

} } // namespace comma { namespace visiting {

template < typename From, typename To = From >
struct intervals
{
    typedef typename bound_traits< From >::type bound_type;
    typedef std::set< std::string > set_t;
    typedef boost::icl::interval_map< bound_t< bound_type >, set_t > map_t;

    const comma::command_line_options& options;
    comma::csv::options csv;
    comma::csv::options ocsv;
    boost::optional< bound_type > empty;
    bool intervals_only;
    bool use_limits;
    map_t map;

    intervals( const comma::command_line_options& options ) : options( options )
                                                            , csv( options )
                                                            , ocsv( options )
                                                            , empty( traits< bound_type >::cast( options.optional< std::string >( "--empty" ) ) )
                                                            , intervals_only( options.exists( "--intervals-only" ) )
                                                            , use_limits( options.exists( "--limits,-l" ) )
    {
        if( csv.fields.empty() ) { csv.fields = comma::join( comma::csv::names< interval_t< From, To > >(), ',' ); }
        if( ocsv.fields.empty() || intervals_only )
        {
            ocsv.fields = comma::join( comma::csv::names< interval_t< From, To > >(), ',' );
            if( ocsv.binary() && intervals_only ) { ocsv.format( comma::csv::format::value< interval_t< From, To > >() ); }
        }
        if( verbose ) { std::cerr << app_name << ": empty: "; empty ? std::cerr << *empty : std::cerr << "<none>"; std::cerr << std::endl; }
    }

    void add( const bound_t< bound_type >& from, const bound_t< bound_type >& to, const std::string& payload )
    {
        set_t s;
        s.insert( payload );
        map += std::make_pair( boost::icl::interval< bound_t< bound_type > >::right_open( from, to ), s );
    }

    void write()
    {
        static comma::csv::output_stream< interval_t< From, To > > ostream( std::cout, ocsv );
        static comma::csv::ascii< from_t< std::string > > from_ascii( ocsv.fields );
        static comma::csv::ascii< to_t< std::string > > to_ascii( ocsv.fields );
        for( typename map_t::iterator it = map.begin(); it != map.end(); ++it )
        {
            bound_t< bound_type > from = it->first.lower();
            bound_t< bound_type > to = it->first.upper();
            interval_t< From, To > interval;
            // bool from_has_value = true;
            // bool to_has_value = true;
            if( from.value ) { interval.from.value = *from.value; }
            else if( use_limits ) { interval.from.value = limits< From >::lowest(); }
            else if( empty ) { interval.from.value = static_cast< From >( *empty ); }
            else if( std::numeric_limits< From >::is_specialized ) { interval.from.value = limits< From >::lowest(); }
            // else { from_has_value = false; }
            if( to.value ) { interval.to.value = *to.value; }
            else if( use_limits ) { interval.to.value = limits< To >::max(); }
            else if( empty ) { interval.to.value = static_cast< To >( *empty ); }
            else if( std::numeric_limits< To >::is_specialized ) { interval.to.value = limits< To >::max(); }
            // else { to_has_value = false; }
            const set_t& s = it->second;
            if( csv.binary() )
            {
                if( intervals_only ) { ostream.write( interval ); ostream.flush(); continue; }
                for( typename set_t::const_iterator v = s.begin(); v != s.end(); ++v ) { ostream.write( interval, *v ); }
                ostream.flush();
            }
            else
            {
                for( typename set_t::const_iterator v = s.begin(); v != s.end(); ++v )
                {
                    std::string payload( intervals_only ? "" : *v );
                    ostream.ascii().ascii().put( interval, payload );
                    // todo: add csv quote option
                    // if( !from_has_value ) { from_ascii.put( from_t< std::string >(), payload ); }
                    // if( !to_has_value ) { to_ascii.put( to_t< std::string >(), payload); }
                    std::cout << payload << std::endl;
                    if( intervals_only ) { break; }
                }
            }
        }
    }

    void run()
    {
        if( debug ) { std::cerr << __PRETTY_FUNCTION__ << std::endl; }
        comma::csv::input_stream< interval_t< From, To > > istream( std::cin, csv );
        comma::csv::ascii< interval_t< std::string > > ascii( csv.fields );

        if( !first_line.empty() )
        {
            interval_t< From, To > interval = comma::csv::ascii< interval_t< From, To > >( csv.fields ).get( first_line );
            bound_t< bound_type > from( LOWER );
            bound_t< bound_type > to( UPPER );
            std::string payload;
            interval_t< std::string > first;
            ascii.get( first, first_line );
            if( !first.from.value.empty() && ( !empty || interval.from.value != *empty ) ) { from.value = interval.from.value; }
            if( !first.to.value.empty() && ( !empty || interval.to.value != *empty  ) ) { to.value = interval.to.value; }
            payload = first_line;
            if( !intervals_only ) { ascii.put( interval_t< std::string >(), payload ); } // blank out interval from payload
            if( verbose ) { std::cerr << app_name << ": from: " << from << " to: " << to << " payload: " << payload << std::endl; }
            add( from, to, payload );
        }

        while( istream.ready() || std::cin.good()  )
        {
            const interval_t< From, To >* interval = istream.read();
            if( !interval ) { break; }
            bound_t< bound_type > from( LOWER );
            bound_t< bound_type > to( UPPER );
            std::string payload;
            if( csv.binary() )
            {
                if( !empty || interval->from.value != *empty ) { from.value = interval->from.value; }
                if( !empty || interval->to.value != *empty ) { to.value = interval->to.value; }
                std::vector< char > buf( istream.binary().last(), istream.binary().last() + istream.binary().size() );
                if( !intervals_only ) { istream.binary().binary().put( interval_t< From, To >(), &buf[0] ); } // blank out interval from payload
                payload = std::string( buf.begin(), buf.end() );
            }
            else
            {
                interval_t< std::string > last;
                ascii.get( last, istream.ascii().last() );
                if( !last.from.value.empty() && ( !empty || interval->from.value != *empty ) ) { from.value = interval->from.value; }
                if( !last.to.value.empty() && ( !empty || interval->to.value != *empty  ) ) { to.value = interval->to.value; }
                std::vector< std::string > buf( istream.ascii().last() );
                if( !intervals_only ) { ascii.put( interval_t< std::string >(), buf ); } // blank out interval from payload
                payload = comma::join( buf, csv.delimiter );
            }
            if( verbose ) { std::cerr << app_name << ": from: " << from << " to: " << to << " payload: " << ( csv.binary() ? "<binary>" : payload ) << std::endl; }
            add( from, to, payload );
        }
        write();
    }
};

template < typename From > void run( const comma::command_line_options& options, const comma::csv::format::types_enum to_type )
{
    switch( to_type )
    {
        case comma::csv::format::int8:          intervals< From, char >( options ).run(); break;
        case comma::csv::format::uint8:         intervals< From, unsigned char >( options ).run(); break;
        case comma::csv::format::int16:         intervals< From, comma::int16 >( options ).run(); break;
        case comma::csv::format::uint16:        intervals< From, comma::uint16 >( options ).run(); break;
        case comma::csv::format::int32:         intervals< From, comma::int32 >( options ).run(); break;
        case comma::csv::format::uint32:        intervals< From, comma::uint32 >( options ).run(); break;
        case comma::csv::format::int64:         intervals< From, comma::int64 >( options ).run(); break;
        case comma::csv::format::uint64:        intervals< From, comma::uint64 >( options ).run(); break;
        case comma::csv::format::char_t:        intervals< From, char >( options ).run(); break;
        case comma::csv::format::float_t:       intervals< From, float >( options ).run(); break;
        case comma::csv::format::double_t:      intervals< From, double >( options ).run(); break;
        default:                                COMMA_THROW( comma::exception, "from/to type mismatch" ); break;
    }
}

int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av );
        verbose = options.exists( "--verbose,-v" );
        debug = options.exists( "--debug" );
        if( options.exists( "--help,-h" ) ) { usage( verbose ); }
        if( options.exists( "--input-fields" ) ) { std::cout << comma::join( comma::csv::names< interval_t< double > >(), ',' ) << std::endl; return 0; }
        if( options.exists( "--output-fields" ) ) { std::cout << comma::join( comma::csv::names< interval_t< double > >(), ',' ) << std::endl; return 0; }
        comma::csv::options csv( options );
        if( csv.fields.empty() ) { csv.fields = comma::join( comma::csv::names< interval_t< double > >(), ',' ); }
        if( !csv.has_field( "from,to" ) ) { COMMA_THROW( comma::exception, "expected from and to fields" ); }
        options.assert_mutually_exclusive( "--binary,--format" );
        if( options.exists( "--binary,-b" ) ) { }
        else if( options.exists( "--format" ) ) { csv.format( options.value< std::string >( "--format" ) ); }
        else
        {
            while( std::cin.good() && first_line.empty() ) { std::getline( std::cin, first_line ); }
            if( first_line.empty() ) { return 0; }
            csv.format( comma::csv::impl::unstructured::guess_format( first_line, csv.delimiter ) );
            if( verbose ) { std::cerr << app_name << ": guessed format: " << csv.format().string() << std::endl;; }
        }
        const std::vector< std::string >& fields = comma::split( csv.fields, ',' );
        unsigned int from_index = 0;
        unsigned int to_index = 1;
        for( unsigned int i = 0; i < fields.size(); ++i ) { if( fields[i] == "from" ) { from_index = i; break; } }
        for( unsigned int i = 0; i < fields.size(); ++i ) { if( fields[i] == "to" ) { to_index = i; break; } }
        const comma::csv::format::types_enum from_type = csv.format().offset( from_index ).type;
        const comma::csv::format::types_enum to_type = csv.format().offset( to_index ).type;
        if( ( ( from_type == comma::csv::format::time || from_type == comma::csv::format::long_time ) && ( to_type != comma::csv::format::time && to_type != comma::csv::format::long_time ) ) ||
          ( ( ( from_type != comma::csv::format::time && from_type != comma::csv::format::long_time ) && ( to_type == comma::csv::format::time || to_type == comma::csv::format::long_time ) ) ) )
        { COMMA_THROW( comma::exception, "from/to type mismatch; time" ); }
        if( ( from_type == comma::csv::format::fixed_string || to_type == comma::csv::format::fixed_string ) && from_type != to_type )
        { COMMA_THROW( comma::exception, "from/to type mismatch; string" ); }
        switch( from_type )
        {
            case comma::csv::format::int8:          run< char >( options, to_type ); break;
            case comma::csv::format::uint8:         run< unsigned char >( options, to_type ); break;
            case comma::csv::format::int16:         run< comma::int16 >( options, to_type ); break;
            case comma::csv::format::uint16:        run< comma::uint16 >( options, to_type ); break;
            case comma::csv::format::int32:         run< comma::int32 >( options, to_type ); break;
            case comma::csv::format::uint32:        run< comma::uint32 >( options, to_type ); break;
            case comma::csv::format::int64:         run< comma::int64 >( options, to_type ); break;
            case comma::csv::format::uint64:        run< comma::uint64 >( options, to_type ); break;
            case comma::csv::format::char_t:        run< char >( options, to_type ); break;
            case comma::csv::format::float_t:       run< float >( options, to_type ); break;
            case comma::csv::format::double_t:      run< double >( options, to_type ); break;
            case comma::csv::format::time:
            case comma::csv::format::long_time:     intervals< boost::posix_time::ptime >( options ).run(); break;
            case comma::csv::format::fixed_string:  intervals< std::string >( options ).run(); break;
            default:                                COMMA_THROW( comma::exception, "unknown type" ); break;
        }
        return 0;
    }
    catch( std::exception& ex )
    {
        std::cerr << app_name << ": " << ex.what() << std::endl;
    }
    catch( ... )
    {
        std::cerr << app_name << ": unknown exception" << std::endl;
    }
    return 1;
}
