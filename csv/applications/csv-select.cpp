// Copyright (c) 2011 The University of Sydney

/// @author vsevolod vlaskine

#include <iostream>
#include <map>
#include <sstream>
#include <unordered_set>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/regex.hpp>
#include <boost/scoped_ptr.hpp>
#include "../../application/command_line_options.h"
#include "../../base/exception.h"
#include "../../csv/stream.h"
#include "../../csv/impl/unstructured.h"
#include "../../math/compare.h"
#include "../../name_value/parser.h"
#include "../../string/string.h"
#include "../../visiting/traits.h"

void usage( bool verbose )
{
    std::cerr << std::endl;
    std::cerr << "find in a file or stream by constraints on a given key" << std::endl;
    std::cerr << std::endl;
    std::cerr << "usage: cat a.csv | csv-select <options> [<key properties>]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "operation options" << std::endl;
    std::cerr << "    --equals=<value>: equals to <value>" << std::endl;
    std::cerr << "    --not-equal=<value>: not equal to <value>" << std::endl;
    std::cerr << "    --greater=<value>: greater than <value>" << std::endl;
    std::cerr << "    --less=<value>: less than <value>" << std::endl;
    std::cerr << "    --from,--greater-or-equal,--ge=<value>: from <value> (inclusive, i.e. greater or equals)" << std::endl;
    std::cerr << "    --to,--less-or-equal,--le=<value>: to <value> (inclusive, i.e. less or equals)" << std::endl;
    std::cerr << "    --regex=<regex>: posix regular expression, string fields only" << std::endl;
    std::cerr << std::endl;
    std::cerr << "      todo: implement a simple boolean expression grammar" << std::endl;
    std::cerr << "input/output control options" << std::endl;
    std::cerr << "    --first-matching: output the first record matching the expression, then exit" << std::endl;
    std::cerr << "    --format=<format>: explicitly specify input format, in case if in ascii mode csv-select guesses incorrectly" << std::endl;
    std::cerr << "    --not-matching: output only not matching records" << std::endl;
    std::cerr << "    --output-all,--all: output all records, append 1 to matching, 0 to not matching; if binary, format of the additional field is 'b'" << std::endl;
    std::cerr << "    --sorted,--input-sorted: a hint that the key column is sorted in ascending order" << std::endl;
    std::cerr << "              todo: support descending order" << std::endl;
    std::cerr << "              attention: this key is applied only to the common constraints, e.g. in the following example" << std::endl;
    std::cerr << "                         --sorted will be applied to the condition --less=2, but NOT to the condition \"x;less=1\"" << std::endl;
    std::cerr << "                         csv-select --less=2 --sorted --fields x \"x;less=1\"" << std::endl;
    std::cerr << "    --strict: if constraint field is not present among fields, exit with error (added for backward compatibility)" << std::endl;
    std::cerr << "    --verbose,-v: more output to stderr" << std::endl;
    std::cerr << "    --or: uses 'or' expression instead of 'and' (default is 'and')" << std::endl;
    std::cerr << std::endl;
    std::cerr << "fields: any non-empty fields will be treated as keys" << std::endl;
    std::cerr << std::endl;
    std::cerr << "csv options" << std::endl;
    std::cerr << comma::csv::options::usage( verbose ) << std::endl;
    std::cerr << std::endl;
    std::cerr << "examples" << std::endl;
    std::cerr << "    cat a.csv | csv-select --fields=,,t --from=20120101T000000" << std::endl;
    std::cerr << "    cat a.csv | csv-select --fields=,,t --from=20120101T000000 --to=20120101T000010 --sorted" << std::endl;
    std::cerr << "    cat xyz.csv | csv-select --fields=x,y,z \"x;from=1;to=2\" \"y;from=-1;to=1.1\" \"z;from=5;to=5.5\"" << std::endl;
    std::cerr << "    cat a.csv | csv-select --fields=t,scalar \"t;from=20120101T000000;sorted\" \"scalar;from=-10;to=20.5\"" << std::endl;
    std::cerr << "    echo hello,world | csv-select --fields=h,w \"h;regex=he.*\"" << std::endl;
    std::cerr << std::endl;
    exit( 0 );
}

bool matches( const std::string& value, const boost::regex& r ) { return boost::regex_match( value, r ); }
template < typename T > bool matches( const T& value, const boost::regex& r ) { COMMA_THROW( comma::exception, "regex implemented only for strings" ); }

template < typename T > static boost::optional< T > get_optional_( const comma::command_line_options& options, const std::string& what ) { return options.optional< T >( what ); }
template <> boost::optional< boost::posix_time::ptime > get_optional_< boost::posix_time::ptime >( const comma::command_line_options& options, const std::string& what )
{
    const boost::optional< std::string >& s = options.optional< std::string >( what );
    if( !s ) { return boost::none; }
    return boost::posix_time::from_iso_string( *s );
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
    boost::optional< boost::regex > regex;
    bool sorted;

    bool empty() const { return !equals && !not_equal && !less && !greater && !from && !to && !regex && !sorted; }

    constraints() : sorted( false ) {}

    constraints( const comma::command_line_options& options ) // quick and dirty
    {
        equals = get_optional_< T >( options, "--equals" );
        not_equal = get_optional_< T >( options, "--not-equal" );
        from = get_optional_< T >( options, "--from,--greater-or-equal,--ge" );
        to = get_optional_< T >( options, "--to,--less-or-equal,--le" );
        less = get_optional_< T >( options, "--less" );
        greater = get_optional_< T >( options, "--greater" );
        const boost::optional< std::string >& s = get_optional_< std::string >( options, "--regex" );
        if( s ) { regex = boost::regex( *s ); }
        sorted = options.exists( "--sorted,--input-sorted" );
    }

    constraints( const std::string& options )
    {
        comma::name_value::map m( options.substr( options.find_first_of( ';' ) + 1 ), ';', '=', true, "equals,not-equal,less,greater,from,greater-or-equal,ge,to,less-or-equal,le,regex,sorted,fields,f,binary,b,delimiter,d,format" ); // quick and dirty, since optional is not well-supported (euphymism for 'buggy') in comma::name_value::parser
        for( const auto& v: m.get() ) // super quick and dirty, suboptimal
        {
            if( v.first == "equals" ) { equals = m.value< T >( "equals" ); }
            else if( v.first == "not-equal" ) { not_equal = m.value< T >( "not-equal" ); }
            else if( v.first == "less" ) { less = m.value< T >( "less" ); }
            else if( v.first == "greater" ) { greater = m.value< T >( "greater" ); } // it was: { equal = m.value< T >( "greater" ); }
            else if( v.first == "from" ) { from = m.value< T >( "from" ); }
            else if( v.first == "greater-or-equal" ) { from = m.value< T >( "greater-or-equal" ); }
            else if( v.first == "ge" ) { from = m.value< T >( "ge" ); }
            else if( v.first == "to" ) { to = m.value< T >( "to" ); }
            else if( v.first == "less-or-equal" ) { to = m.value< T >( "less-or-equal" ); }
            else if( v.first == "le" ) { to = m.value< T >( "le" ); }
            else if( v.first == "regex" ) { regex = boost::regex( m.value< std::string >( "regex" ) ); }
            else if( v.first == "sorted" ) { sorted = true; }
        }
    }

    bool is_a_match( const T& t ) const // quick and dirty, implement a proper expression parser
    {
        return    ( !equals || comma::math::equal( *equals, t ) )
               && ( !not_equal || !comma::math::equal( *not_equal, t ) )
               && ( !from || !comma::math::less( t, *from ) )
               && ( !to || !comma::math::less( *to, t ) )
               && ( !less || comma::math::less( t, *less ) )
               && ( !greater || comma::math::less( *greater, t ) )
               && ( !regex || matches( t, *regex ) );
    }

    bool done( const T& t ) const // quick and dirty
    {
        if( !sorted ) { return false; }
        if( to && comma::math::less( *to, t ) ) { return true; }
        if( less && !comma::math::less( t, *less ) ) { return true; }
        if( equals && comma::math::less( *equals, t ) ) { return true; }
        // todo: more?
        return false;
    }
};

static bool default_constraints_empty( const comma::command_line_options& options ) // quick and dirty
{
    static bool empty = constraints< std::string >( options ).empty();
    return empty;
}

template < typename T >
struct constrained
{
    T value;
    std::vector< ::constraints< T > > constraints;

    bool is_a_match( bool is_or = false ) const
    {
        if( is_or )
        {
            for( unsigned int i = 0; i < constraints.size(); ++i ) { if( this->constraints[i].is_a_match( value ) ) { return true; } }
            return false;
        }
        for( unsigned int i = 0; i < constraints.size(); ++i ) { if( !this->constraints[i].is_a_match( value ) ) { return false; } }
        return true;
    }

    bool done( bool is_or = false ) const
    {
        if( constraints.empty() ) { return false; }
        if( is_or )
        {
            for( unsigned int i = 0; i < constraints.size(); ++i ) { if( this->constraints[i].done( value ) ) { return true; } }
            return false;
        }
        for( unsigned int i = 0; i < constraints.size(); ++i ) { if( !this->constraints[i].done( value ) ) { return false; } }
        return true;
    }
};

struct input_t
{
    std::vector< constrained< boost::posix_time::ptime > > time;
    std::vector< constrained< double > > doubles;
    std::vector< constrained< std::string > > strings;

    bool is_a_match( bool is_or ) const
    {
        if( is_or )
        {
            for( unsigned int i = 0; i < time.size(); ++i ) { if( time[i].is_a_match( is_or ) ) { return true; } }
            for( unsigned int i = 0; i < doubles.size(); ++i ) { if( doubles[i].is_a_match( is_or ) ) { return true; } }
            for( unsigned int i = 0; i < strings.size(); ++i ) { if( strings[i].is_a_match( is_or ) ) { return true; } }
            return false;
        }
        else
        {
            for( unsigned int i = 0; i < time.size(); ++i ) { if( !time[i].is_a_match() ) { return false; } }
            for( unsigned int i = 0; i < doubles.size(); ++i ) { if( !doubles[i].is_a_match() ) { return false; } }
            for( unsigned int i = 0; i < strings.size(); ++i ) { if( !strings[i].is_a_match() ) { return false; } }
            return true;
        }
    }

    bool done( bool is_or ) const
    {
        if( is_or )
        {
            for( unsigned int i = 0; i < time.size(); ++i ) { if( !time[i].done( is_or ) ) { return false; } }
            for( unsigned int i = 0; i < doubles.size(); ++i ) { if( !doubles[i].done( is_or ) ) { return false; } }
            for( unsigned int i = 0; i < strings.size(); ++i ) { if( !strings[i].done( is_or ) ) { return false; } }
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

template < typename T > struct traits< constrained< T > >
{
    template < typename K, typename V > static void visit( const K&, const constrained< T >& p, V& v ) { v.apply( "value", p.value ); }
    template < typename K, typename V > static void visit( const K&, constrained< T >& p, V& v ) { v.apply( "value", p.value ); }
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

static comma::csv::options csv;
static input_t input;
static std::vector< std::string > fields;
typedef std::multimap< std::string, std::string > constraints_map_t;
static constraints_map_t constraints_map;

template < typename T > static constrained< T > make_value( unsigned int i, const comma::command_line_options& options )
{
    constrained< T > v;
    for( auto r = constraints_map.equal_range( fields[i] ); r.first != r.second; ++r.first ) { v.constraints.push_back( constraints< T >( r.first->second ) ); }
    static constraints< T > common_constraints( options ); // quick and dirty
    if( !common_constraints.empty() ) { v.constraints.push_back( common_constraints ); }
    return v;
}

static void init_input( const comma::csv::format& format, const comma::command_line_options& options )
{
    if( fields.empty() ) { for( unsigned int i = 0; i < format.count(); ++i ) { fields.push_back( "v" ); } }
    for( unsigned int i = 0; i < fields.size(); ++i )
    {
        if( comma::strip( fields[i], ' ' ).empty() ) { continue; }
        if( default_constraints_empty( options ) && constraints_map.find( fields[i] ) == constraints_map.end() ) { continue; }
        switch( format.offset( i ).type )
        {
            case comma::csv::format::time:
            case comma::csv::format::long_time:
                input.time.push_back( make_value< boost::posix_time::ptime >( i, options ) );
                fields[i] = "t[" + boost::lexical_cast< std::string >( input.time.size() - 1 ) + "]/value";
                break;
            case comma::csv::format::fixed_string:
                input.strings.push_back( make_value< std::string >( i, options ) );
                fields[i] = "strings[" + boost::lexical_cast< std::string >( input.strings.size() - 1 ) + "]/value";
                break;
            default:
                input.doubles.push_back( make_value< double >( i, options ) );
                fields[i] = "doubles[" + boost::lexical_cast< std::string >( input.doubles.size() - 1 ) + "]/value";
                break;
        }
    }
    csv.fields = comma::join( fields, ',' );
}

int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av, usage );
        bool is_or = options.exists( "--or" );
        csv = comma::csv::options( options );
        fields = comma::split( csv.fields, ',' );
        if( fields.size() == 1 && fields[0].empty() ) { fields.clear(); }
        std::vector< std::string > unnamed = options.unnamed( "--first-matching,--or,--sorted,--input-sorted,--not-matching,--output-all,--all,--strict,--verbose,-v,--flush"
                                                            , "--equals,--not-equal,--less,--greater,--from,--greater-or-equal,--ge,--to,--less-or-equal,--le,--regex,--fields,-f,--binary,-b,--format,--delimiter,-d,--precision" );
        //for( unsigned int i = 0; i < unnamed.size(); constraints_map.insert( std::make_pair( comma::split( unnamed[i], ';' )[0], unnamed[i] ) ), ++i );
        bool strict = options.exists( "--strict" );
        bool first_matching = options.exists( "--first-matching" );
        bool not_matching = options.exists( "--not-matching" );
        bool all = options.exists( "--output-all,--all" );
        for( unsigned int i = 0; i < unnamed.size(); ++i )
        {
            std::string field = comma::split( unnamed[i], ';' )[0];
            bool found = false;
            for( unsigned int j = 0; j < fields.size() && !found; found = field == fields[j], ++j );
            if( found ) { constraints_map.insert( std::make_pair( field, unnamed[i] ) ); continue; }
            if( strict ) { std::cerr << "csv-select: on constraint: \"" << unnamed[i] << "\" field \"" << field << "\" not found in fields: " << csv.fields << std::endl; return 1; }
            std::cerr << "csv-select: warning: on constraint: \"" << unnamed[i] << "\" field \"" << field << "\" not found in fields: " << csv.fields << std::endl;
        }
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
                if( !p || p->done( is_or ) ) { break; }
                char match = ( p->is_a_match( is_or ) == !not_matching ) ? 1 : 0;
                if( match || all )
                {
                    std::cout.write( istream.last(), csv.format().size() );
                    if( all ) { std::cout.write( &match, 1 ); }
                    if( csv.flush ) { std::cout.flush(); }
                    if( first_matching ) { break; }
                }
            }
        }
        else
        {
            std::string line;
            while( std::cin.good() && !std::cin.eof() )
            {
                std::getline( std::cin, line );
                line = comma::strip( line, '\r' ); // windows, sigh...
                if( !line.empty() ) { break; }
            }
            if( line.empty() ) { return 0; }
            comma::csv::format format = options.exists( "--format" )
                                      ? comma::csv::format( options.value< std::string >( "--format" ) )
                                      : comma::csv::impl::unstructured::guess_format( line );
            if( !options.exists( "--format" ) ) { std::cerr << "csv-select: guessed format from the first input line: " << format.string() << "; if you think the guess is wrong, please specify --format" << std::endl; }
            init_input( format, options );
            comma::csv::ascii_input_stream< input_t > istream( std::cin, csv, input );
            // todo: quick and dirty: no time to debug why the commented section does not work (but that's the right way)
            std::istringstream iss( line );
            comma::csv::ascii_input_stream< input_t > isstream( iss, csv, input );
            const input_t* p = isstream.read();
            if( !p || p->done( is_or ) ) { return 0; }
            bool match = p->is_a_match( is_or ) == !not_matching;
            if( match || all )
            {
                std::cout << line;
                if( all ) { std::cout << csv.delimiter << match; }
                std::cout << std::endl;
                if( first_matching ) { return 0; }
            }
            while( istream.ready() || ( std::cin.good() && !std::cin.eof() ) )
            {
                const input_t* p = istream.read();
                if( !p || p->done( is_or ) ) { break; }
                bool match = p->is_a_match( is_or ) == !not_matching;
                if( match || all )
                {
                    std::cout << comma::join( istream.last(), csv.delimiter );
                    if( all ) { std::cout << csv.delimiter << match; }
                    std::cout << std::endl;
                    if( first_matching ) { return 0; }
                }
            }
        }
        return 0;
    }
    catch( std::exception& ex ) { std::cerr << "csv-select: caught: " << ex.what() << std::endl; }
    catch( ... ) { std::cerr << "csv-select: unknown exception" << std::endl; }
    return 1;
}
