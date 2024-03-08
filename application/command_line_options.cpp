// Copyright (c) 2011 The University of Sydney
// Copyright (c) 2022 Vsevolod Vlaskine

/// @author vsevolod vlaskine

#include <algorithm>
#include <array>
#include <sstream>
#include <set>
#include <unordered_map>
#include <boost/bind/bind.hpp>
#include <boost/config/warning_disable.hpp>
//#include <boost/filesystem.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/null.hpp>
#include <boost/optional.hpp>
#include <boost/regex.hpp>
#include <boost/spirit/include/qi.hpp>
#include "../io/terminal.h"
#include "../string/split.h"
#include "../base/exception.h"
#include "command_line_options.h"

namespace comma {

namespace application { namespace detail {

static std::string name;
static unsigned int verbosity_level{0};
static bool titlebar{false};
static boost::iostreams::stream< boost::iostreams::null_sink > null_ostream( ( boost::iostreams::null_sink() ) );

} } // namespace application { namespace detail {

unsigned int verbosity::level() { return comma::application::detail::verbosity_level; }

unsigned int verbosity::from_string( const std::string& s )
{
    return   s == "none"                     ? verbosity::none
           : s == "low"    || s == "error"   ? verbosity::low
           : s == "medium" || s == "warning" ? verbosity::medium
           : s == "high"   || s == "info"    ? verbosity::high
           : s == "debug"  || s == "extreme" ? verbosity::extreme
           : boost::lexical_cast< unsigned int >( s );
}

const std::string verbosity::to_string( unsigned int v )
{
    static const std::array< std::string, 5 > s{{ "", "low", "medium", "high", "extreme" }};
    return v < s.size() ? s[v] : ""; // output lexical cast?
}

std::string verbosity::usage()
{
    const char* s = R"verbosity(verbosity options
    --titlebar,--tb; output terminal-destined messages to terminal title bar, default: stderr
    --titlebar-application-name,--tbn; on application start, set terminal title bar to application name
    --verbose,-v; more output on stderr, same as --verbosity=1
    --verbosity=<n>; default=0; verbosity level from 0 to 5 or 'none'(0), 'low'|'error'(1), 'medium'|'warning'(2), 'high'|'info'(3), 'extreme'|'debug'(4)
    -v,-vv,-vvv,-vvvv,-vvvvv; same as --verbosity from 1 to 5
)verbosity";
    return s;
}

std::ostream& say( std::ostream& os, unsigned int verbosity, const std::string& prefix )
{
    return ( verbosity > comma::application::detail::verbosity_level ? comma::application::detail::null_ostream : os ) << comma::application::detail::name << ": " << ( prefix.empty() ? std::string() : ( prefix + ": " ) );
}

void command_line_options::_init_verbose( const std::string& path )
{
    comma::application::detail::verbosity_level = verbosity::from_string( value< std::string >( "--verbosity", exists( "--verbose,-v" ) ? "1" : "0" ) );
    static const std::array< std::string, 5 > v{{ "-vvvvv", "-vvvv", "-vvv", "-vv", "-v" }}; // add more verbosity levels if some strange people need them
    for( unsigned int i = 0; i < v.size() && comma::application::detail::verbosity_level + i < v.size(); ++i )
    {
        if( exists( v[i] ) ) { comma::application::detail::verbosity_level = v.size() - i; break; }
    }
    comma::verbose.init( comma::application::detail::verbosity_level > 0, path ); // todo: deprecate, use comma::say() and comma::saymore() instead
    comma::application::detail::name = comma::split( path, '/' ).back(); // boost::filesystem::basename( path );
    comma::application::detail::titlebar = exists( "--titlebar,--tb" );
    if( exists( "--titlebar-application-name,--tbn" ) ) { comma::io::terminal::titlebar_ostream s; s << comma::application::detail::name; }
}

command_line_options::command_line_options( int argc, char ** argv, boost::function< void( bool ) > usage, boost::function< void( int, char** ) > bash_completion )
{
    argv_.resize( argc );
    for( int i = 0; i < argc; ++i ) { argv_[i] = argv[i]; }
    _fill_map( argv_ );
    _init_verbose( argv[0] );
    if( bash_completion && exists( "--bash-completion" ) ) { bash_completion( argc, argv ); exit( 0 ); }
    if( usage && exists( "--help,-h" ) ) { usage( comma::application::detail::verbosity_level > 0 ); exit( 0 ); }
}

command_line_options::command_line_options( const std::vector< std::string >& argv, boost::function< void( bool ) > usage )
    : argv_( argv )
{
    _fill_map( argv_ );
    _init_verbose( argv[0] );
    if( usage && exists( "--help,-h" ) ) { usage( comma::application::detail::verbosity_level > 0 ); exit( 1 ); }
}

std::string command_line_options::escaped( const std::string& s ) // quick and dirty
{
    static const boost::regex r( "\\\"" );
    //return s.find_first_of( "; \t\n&<>|$#*?()[]{}\'\"" ) == std::string::npos ? s : boost::regex_replace( s, r, "\\\\\"" );
    return boost::regex_replace( s, r, "\\\\\"" );
}

std::string command_line_options::string() const
{
    std::ostringstream out;
    std::string space;
    for( std::size_t i = 0; i < argv_.size(); ++i )
    {
        out << space << '"' << escaped( argv_[i] ) << '"';
        space = " ";
    }
    return out.str();
}

command_line_options::command_line_options( const command_line_options& rhs ) { operator=( rhs ); }

const std::vector< std::string >& command_line_options::argv() const { return argv_; }

bool command_line_options::exists( const std::string& name ) const
{
    std::vector< std::string > names = comma::split( name, ',' );
    for( const std::string& n: names ) { if( map_.find( n ) != map_.end() ) { return true; } }
    return false;
}

std::vector< std::string > command_line_options::unnamed( const std::string& valueless_options, const std::string& options_with_values ) const
{

    std::vector< std::string > valueless{ "--verbose", "-v", "-vv", "-vvv", "-vvvv", "-vvvvv", "--titlebar", "--tb", "--titlebar-application-name", "--tbn", "" };
    if( !valueless_options.empty() ) { valueless = split( valueless_options + ",--verbose,-v,-vv,-vvv,-vvvv,-vvvvv,--titlebar,--tb,--titlebar-application-name,--tbn", ',' ); }
    std::vector< std::string > valued = split( options_with_values, ',' );
    std::vector< std::string > w;
    for( unsigned int i = 1; i < argv_.size(); ++i )
    {
        bool is_valueless = false;
        for( unsigned int k = 0; !is_valueless && k < valueless.size(); ++k ) { is_valueless = boost::regex_match( argv_[i], boost::regex( valueless[k] ) ); }
        if( is_valueless ) { continue; }
        bool is_valued = false;
        bool has_equal_sign = false;
        for( unsigned int j = 0; !is_valued && j < valued.size(); ++j )
        {
            has_equal_sign = boost::regex_match( argv_[i], boost::regex( valued[j] + "=.*" ) );
            is_valued = boost::regex_match( argv_[i], boost::regex( valued[j] ) ) || has_equal_sign;
        }
        if( is_valued ) { if( !has_equal_sign ) { ++i; } continue; }
        w.push_back( argv_[i] );
    }
    return w;
}

std::vector< std::string > command_line_options::names() const { return names_; }

void command_line_options::_fill_map( const std::vector< std::string >& v )
{
    for( std::size_t i = 1; i < v.size(); ++i )
    {
        if( v[i].length() < 2 || v[i].at( 0 ) != '-') { continue; }
        std::string name;
        boost::optional< std::string > value;
        std::size_t equal = v[i].find_first_of( '=' );
        if( equal == std::string::npos )
        {
            name = v[i];
            if( ( i + 1 ) < v.size() ) { value = v[ i + 1 ]; }
        }
        else
        {
            name = v[i].substr( 0, equal );
            if( ( equal + 1 ) < v[i].length() ) { value = v[i].substr( equal + 1 ); }
        }
//         if( v[i].at( 1 ) == '-' )
//         {
//             if( v[i].length() < 3 ) { continue; }
//             if( equal == std::string::npos )
//             {
//                 name = v[i];
//                 // check if the option has a value, i.e. not starting with '-' unless it is a negative number
//                 if( ( i + 1 < v.size() ) && ( ( v[i+1][0] != '-' ) || isdigit( v[i+1][1] ) ) ) { value = v[ i + 1 ]; }
//             }
//             else
//             {
//                 name = v[i].substr( 0, equal );
//                 value = v[i].substr( equal + 1 );
//             }
//         }
//         else
//         {
//             name = v[i];
//             if( i + 1 < v.size() ) { value = v[ i + 1 ]; }
//         }
        std::vector< std::string >& values = map_[name];
        if( value ) { values.push_back( *value ); }
        if( !name.empty() ) { names_.push_back( name ); }
    }
}

void command_line_options::assert_mutually_exclusive( const std::string& names ) const
{
    const std::vector< std::string >& v = comma::split( names, ',' );
    std::size_t count = 0;
    for( std::size_t i = 0; i < v.size(); ++i )
    {
        count += exists( v[i] );
        if( count > 1 ) { COMMA_THROW( comma::exception, "options " << names << " are mutually exclusive" ); }
    }
}

void command_line_options::assert_exists_if( const std::string& first, const std::string& second ) const
{
    if( !exists( first ) ) { return; }
    for( const auto& o: comma::split( second, ',', true ) )
    {
        if( !exists( o ) ) { COMMA_THROW( comma::exception, "if " << first << ", please specify " << o ); }
    }
}

void command_line_options::assert_exists( const std::string& names ) const { if( !exists( names ) ) { COMMA_THROW( comma::exception, "please specify one of the following: " << names  ); } }

void command_line_options::assert_mutually_exclusive( const std::string& first, const std::string& second ) const
{
    const std::vector< std::string >& v = comma::split( first, ',' );
    const std::string* f = NULL;
    for( std::size_t i = 0; i < v.size(); ++i ) { if( exists( v[i] ) ) { f = &v[i]; break; } }
    if( !f ) { return; }
    const std::vector< std::string >& w = comma::split( second, ',' );
    for( std::size_t i = 0; i < w.size(); ++i ) { if( exists( w[i] ) ) { COMMA_THROW( comma::exception, "options " << *f << " and " << w[i] << " are mutually exclusive" ); } }
}

void command_line_options::assert_valid( const std::vector< description >& d, bool unknown_options_invalid )
{
    for( unsigned int i = 0; i < d.size(); ++i ) { d[i].assert_valid( *this ); }
    if( !unknown_options_invalid ) { return; }
    std::unordered_map< std::string, bool > m; // real quick and dirty, just to make it work
    for( unsigned int i = 0; i < d.size(); ++i ) { for( unsigned int j = 0; j < d[i].names.size(); ++j ) { m[ d[i].names[j] ] = d[i].has_value; } }
    for( unsigned int i = 1; i < argv_.size(); ++i )
    {
        std::string option_name = comma::split( argv_[i], '=' )[0];
        if( !boost::regex_match( option_name, boost::regex( "-.+" ) ) ) { continue; }
        auto it = m.find( option_name );
        if( it == m.end() ) { COMMA_THROW( comma::exception, "unknown option " << option_name ); }
        if( it->second ) { ++i; }
    }
}

namespace impl {
    namespace qi = boost::spirit::qi;
    namespace ascii = boost::spirit::ascii;

    typedef std::string::const_iterator iterator;
    typedef comma::command_line_options::description description_t;

    static void set_( std::string& s, const std::string& t ) { s = t; }
    static void push_back_( std::vector< std::string >& v, const std::string& t ) { v.push_back( t ); }
    static void got_value( description_t& d, const std::string& ) { d.has_value = true; d.is_optional = false; }
    static void got_optional_value( description_t& d, const std::string& ) { d.has_value = true; d.is_optional = true; }
    static void got_default_value( description_t& d, const std::string& s ) { if( d.has_value ) { d.default_value = s; d.is_optional = true; } }

    comma::command_line_options::description from_string_impl_( const std::string& s )
    {
        typedef qi::rule< iterator, std::string(), ascii::space_type > rule;
        rule string = qi::lexeme[ +( boost::spirit::qi::alpha | boost::spirit::qi::digit | ascii::char_( '-' ) | ascii::char_( '_' ) ) ];
        //qi::rule< iterator, std::string(), ascii::space_type > long_name = ascii::char_( '-' ) >> ascii::char_( '-' ) >> string;
        rule name = ascii::char_( '-' ) >> string;
        rule value = '<' >> string >> '>';
        rule optional_value = qi::lit( "[<" ) >> string >> qi::lit( ">]" );
        rule double_quoted = qi::lit( "\"" ) >> qi::no_skip[ *( ( '\\' > ascii::char_( "\"" ) ) | ~ascii::char_( "\"" ) ) ] >> qi::lit( "\"" );
        rule single_quoted = qi::lit( "'" ) >> qi::no_skip[ *( ( '\\' > ascii::char_( "\'" ) ) | ~ascii::char_( "\'" ) ) ] >> qi::lit( "'" );
        rule default_value = qi::lit( "default=" ) >> ( double_quoted | single_quoted | qi::no_skip[ *( ~ascii::space - ascii::char_( ";" ) ) ] );
        rule help = qi::no_skip[ *ascii::char_ ];

        description_t d;
        bool r = boost::spirit::qi::phrase_parse( s.begin()
                                                , s.end()
                                                ,      name[ boost::bind( push_back_, boost::ref( d.names ), boost::placeholders::_1 ) ]
                                                    >> *( ',' >> name[ boost::bind( push_back_, boost::ref( d.names ), boost::placeholders::_1 ) ] )
                                                    >> -( '=' >> ( value[ boost::bind( got_value, boost::ref( d ), boost::placeholders::_1 ) ]
                                                                | optional_value[ boost::bind( got_optional_value, boost::ref( d ), boost::placeholders::_1 ) ] ) )
                                                    >> -( ';' >> default_value[ boost::bind( got_default_value, boost::ref( d ), boost::placeholders::_1 ) ] )
                                                    >> -( ';' >> *( ascii::space ) >> help[ boost::bind( set_, boost::ref( d.help ), boost::placeholders::_1 ) ] )
                                                    >> qi::eoi
                                                , ascii::space );
        if( !r ) { COMMA_THROW( comma::exception, "invalid option description: \"" << s << "\"" ); }
        if( d.names[0].size() < 3 || d.names[0][0] != '-' || d.names[0][1] != '-' ) { COMMA_THROW( comma::exception, "expected full name (e.g. --filename), got \"" << d.names[0] << "\" in: \"" << s << "\"" ); }
        if( !d.has_value ) { d.is_optional = true; d.default_value.reset(); }
        return d;
    }
} // namespace impl {

template < typename T > std::string to_string_( const T& v ) { return boost::lexical_cast< std::string >( v ); }

std::string to_string_( bool s ) { return s ? "true" : "false"; }

void command_line_options::description::assert_valid( const command_line_options& options ) const
{
    if( !has_value ) { return; }
    if( is_optional || default_value ) { return; }
    for( unsigned int i = 0; i < names.size(); ++i ) { if( options.exists( names[i] ) ) { return; } }
    COMMA_THROW( comma::exception, "please specify " << names[0] );
}

bool command_line_options::description::valid( const command_line_options& options ) const throw()
{
    if( is_optional || default_value ) { return true; }
    try { for( unsigned int i = 0; i < names.size(); ++i ) { if( options.exists( names[i] ) ) { return true; } } } catch( ... ) {}
    return false;
}

namespace impl { command_line_options::description from_string_impl_( const std::string& s ); }

command_line_options::description command_line_options::description::from_string( const std::string& s ) { return impl::from_string_impl_( s ); } // real quick and dirty, just to get it working

std::string command_line_options::description::as_string() const
{
    std::string s = names[0];
    for( unsigned int i = 1; i < names.size(); ++i ) { s += ',' + names[i]; }
    if( has_value )
    {
        s += '=';
        s += is_optional ? "[<value>]" : "<value>";
        if( default_value ) { s += "; default=" + *default_value; }
    }
    s += "; ";
    s += help;
    return s;
}

std::string comma::command_line_options::description::usage()
{
    std::ostringstream oss;
    oss << "    option description: <name>[,<name>,<name>...][=[<value>]][; default=<value>][; help], e.g:" << std::endl;
    description has_no_value;
    has_no_value.names.push_back( "--verbose" );
    has_no_value.names.push_back( "-v" );
    has_no_value.help = "option with no value";
    description has_mandatory_value;
    has_mandatory_value.names.push_back( "--filename" );
    has_mandatory_value.names.push_back( "--file" );
    has_mandatory_value.names.push_back( "--f" );
    has_mandatory_value.has_value = true;
    has_mandatory_value.help = "mandatory option with value and no default";
    description has_optional_value;
    has_optional_value.names.push_back( "--output-file" );
    has_optional_value.names.push_back( "-o" );
    has_optional_value.has_value = true;
    has_optional_value.is_optional = true;
    has_optional_value.help = "optional option with value and no default";
    description has_default_value;
    has_default_value.names.push_back( "--threshold" );
    has_default_value.names.push_back( "-t" );
    has_default_value.has_value = true;
    has_default_value.default_value = "20";
    has_default_value.help = "option with a default value";
    oss << "        " << has_no_value.as_string() << std::endl;
    oss << "        " << has_mandatory_value.as_string() << std::endl;
    oss << "        " << has_optional_value.as_string() << std::endl;
    oss << "        " << has_default_value.as_string() << std::endl;
    return oss.str();
}

} // namespace comma {
