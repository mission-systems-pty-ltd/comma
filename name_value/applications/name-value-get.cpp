// Copyright (c) 2011 The University of Sydney

/// @author vsevolod vlaskine

#include <iostream>
#include <boost/property_tree/info_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/regex.hpp>
#include "../../base/exception.h"
#include "../../application/command_line_options.h"
#include "../../name_value/ptree.h"
#include "../../name_value/serialize.h"
#include "../../xpath/xpath.h"

static const std::string regex_characters_ =  ".{}()\\*+?|^$";

static void usage( bool verbose = false )
{
    std::cerr << std::endl;
    std::cerr << "take json, xml, or path-value formatted data on stdin and output value at given path on stdout" << std::endl;
    std::cerr << std::endl;
    std::cerr << "usage: cat data.xml | name-value-get <paths> [<options>]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "<paths>: x-path, e.g. \"command/type\" or posix regular expressions" << std::endl;
    std::cerr << "    if <paths> doesn't contain any of regex characters: \"" << regex_characters_ << "\" it will be treated as x-path" << std::endl;
    std::cerr << "    x-path may contain array index e.g. y[0]/x/z[1]=\"a\"" << std::endl;
    std::cerr << "    if you want to force using regular expressions, use --regex option (see below)" << std::endl;
    std::cerr << "    rationale: there is no way to tell whether a[12] is the 12th element of array or a regular expression" << std::endl;
    std::cerr << "    warning: regular expression matching does not work very well with indexed names; use grep instead" << std::endl;
    std::cerr << std::endl;
    std::cerr << "data options" << std::endl;
    std::cerr << "    --from <format>: input format; if this option is omitted, input format will be guessed (only for json, xml, and path-value)" << std::endl;
    std::cerr << "    --to <format>: output format; default: path-value" << std::endl;
    std::cerr << "    --regex: add square brackets \"[]\" to regex characters; when not specified indexed path can be used e.g. x/y[0]" << std::endl;
    std::cerr << "formats" << std::endl;
    std::cerr << "    info: info data (see boost::property_tree)" << std::endl;
    std::cerr << "    ini: ini data" << std::endl;
    std::cerr << "    json: json data" << std::endl;
    std::cerr << "    xml: xml data" << std::endl;
    std::cerr << "    path-value: path=value-style data; e.g. x/a=1,x/b=2,y=3" << std::endl;
    std::cerr << std::endl;
    std::cerr << "name/path-value options" << std::endl;
    std::cerr << "    --equal-sign,-e=<equal sign>: default '='" << std::endl;
    std::cerr << "    --delimiter,-d=<delimiter>: default ','" << std::endl;
    std::cerr << "    --minify: if present, output minified json" << std::endl;
    std::cerr << "    --no-brackets: show indices as path elements e.g. y/0/x/z/1=\"a\"" << std::endl;
    std::cerr << "                   by default array items will be shown with index e.g. y[0]/x/z[1]=\"a\"" << std::endl;
    std::cerr << "    --output-path: if path-value, output path (for regex)" << std::endl;
    std::cerr << "    --quote-numbers,--quote: force quoting the numbers and booleans" << std::endl;
    std::cerr << "                             unfortunately, historically path-value and xml quote numbers by default and json unquotes numbers by default" << std::endl;
    std::cerr << "                             this default behaviour is left unchaged to keep backward compatibility" << std::endl;
    std::cerr << "    --unquote-numbers,--unquote: unquote the numbers and booleans" << std::endl;
    std::cerr << std::endl;
    std::cerr << "path-value output options" << std::endl;
    std::cerr << "    --take-last: if paths are repeated, take last path=value" << std::endl;
    std::cerr << "    --verify-unique,--unique-input: ensure that all input paths are unique (takes precedence over --take-last)" << std::endl;
    std::cerr << "    warning: if paths are repeated, output value selected from these inputs in not deterministic" << std::endl;
    std::cerr << std::endl;
    std::cerr << "data flow options:" << std::endl;
    std::cerr << "    --linewise,-l: if present, treat each input line as a record" << std::endl;
    std::cerr << "                   if absent, treat all of the input as one record" << std::endl;
    std::cerr << std::endl;
    std::cerr << std::endl;
    exit( 1 );
}

static char equal_sign;
static char path_value_delimiter;
static bool linewise;
static bool minify;
static bool option_regex;
static bool output_path;
static bool unquote_numbers;
static bool quote_numbers;
typedef comma::property_tree::path_mode path_mode;
static path_mode indices_mode = comma::property_tree::disabled;
static comma::property_tree::path_value::check_repeated_paths check_type( comma::property_tree::path_value::no_check );

enum Types { ini, info, json, xml, path_value, void_t };

template < Types Type > struct traits {};

template <> struct traits< void_t >
{
    static void input( std::istream& is, boost::property_tree::ptree& ptree )  { comma::property_tree::from_unknown( is, ptree, check_type, equal_sign, path_value_delimiter  ); }
};

template <> struct traits< ini >
{
    static void input( std::istream& is, boost::property_tree::ptree& ptree ) { boost::property_tree::read_ini( is, ptree ); }
    static void output( std::ostream& os, const boost::property_tree::ptree& ptree, const std::string& ) { boost::property_tree::write_ini( os, ptree ); }
};

template <> struct traits< info >
{
    static void input( std::istream& is, boost::property_tree::ptree& ptree ) { boost::property_tree::read_info( is, ptree ); }
    static void output( std::ostream& os, const boost::property_tree::ptree& ptree, const std::string& ) { boost::property_tree::write_info( os, ptree ); }
};

template <> struct traits< json >
{
    static void input( std::istream& is, boost::property_tree::ptree& ptree ) { boost::property_tree::read_json( is, ptree ); }
    static void output( std::ostream& os, const boost::property_tree::ptree& ptree, const std::string& ) { comma::name_value::impl::write_json( os, ptree, minify, !quote_numbers ); }
};

template <> struct traits< xml >
{
    static void input( std::istream& is, boost::property_tree::ptree& ptree ) { boost::property_tree::read_xml( is, ptree ); }
    static void output( std::ostream& os, const boost::property_tree::ptree& ptree, const std::string& ) { boost::property_tree::write_xml( os, ptree ); }
};

template <> struct traits< path_value > // quick and dirty
{
    static void input( std::istream& is, boost::property_tree::ptree& ptree )
    {
        if( !linewise ) { comma::property_tree::from_path_value( is, ptree, check_type, equal_sign, path_value_delimiter, true ); return; }
        std::string line;
        std::getline( is, line );
        ptree = comma::property_tree::from_path_value_string( line, equal_sign, path_value_delimiter, check_type, true );
    }
    static void output( std::ostream& os, const boost::property_tree::ptree& ptree, const std::string& path )
    {
        static bool first = true; // todo: will not work linewise, fix
        if( !first ) { std::cout << path_value_delimiter; }
        first = false;
        comma::property_tree::to_path_value( os, ptree, indices_mode, equal_sign, path_value_delimiter, output_path ? path : std::string(), unquote_numbers );
    }
};

static std::vector< std::string > path_strings;
static std::vector< boost::property_tree::ptree::path_type > paths;
static std::vector< boost::optional< boost::regex > > path_regex;
static void ( * input )( std::istream& is, boost::property_tree::ptree& ptree );
static void ( * output )( std::ostream& is, const boost::property_tree::ptree& ptree, const std::string& );
static bool output_last_line_break = false;

static void match_( std::ostream& os, const boost::property_tree::ptree& ptree )
{
    static const boost::property_tree::ptree::path_type empty;
    bool found = false;
    for( std::size_t i = 0; i < paths.size(); ++i )
    {
        boost::optional< const boost::property_tree::ptree& > child = comma::property_tree::get_tree(ptree, path_strings[i]);
        if( !child ) { continue; }
        boost::optional< std::string > value = child->get_optional< std::string >( empty );
        if( value && !value->empty() ) // todo! homogenise single values with path-value pairs; currently the following example will output junk: ( echo x/z=0; echo y=1 ) | name-value-get x y
        {
            if( found ) { os << path_value_delimiter; }
            if( output_path ) { os << path_strings[i] << equal_sign; }
            os << *value;
        }
        else
        {
            output( os, *child, path_strings[i] );
        }
        found = true;
    }
    if( found && output_last_line_break ) { os << std::endl; }
}

static bool traverse_( std::ostream& os, const boost::property_tree::ptree& ptree, boost::property_tree::ptree::const_iterator it, comma::xpath& path )
{
    static const boost::property_tree::ptree::path_type empty;
    static bool found = false;
    path /= it->first;
    const std::string& s = path.to_string( '/' ); // quick and dirty
    for( std::size_t i = 0; i < paths.size(); ++i ) // todo: quick and dirty: can prune much earlier, i guess...
    {
        if( path_regex[i] ) { if( !boost::regex_match( s, *path_regex[i] ) ) { continue; } }
        else if( s != path_strings[i] ) { continue; }
        boost::optional< const boost::property_tree::ptree& > child = ptree.get_child_optional( path.to_string( '.' ) ); // quick and dirty, watch performance
        if( !child ) { continue; }
        boost::optional< std::string > value = child->get_optional< std::string >( empty );
        if( value && !value->empty() ) // todo! homogenise single values with path-value pairs; currently the following example will output junk: ( echo x/z=0; echo y=1 ) | name-value-get x y
        {
            if( found ) { os << path_value_delimiter; }
            if( output_path ) { os << s << equal_sign; }
            os << *value;
        }
        else
        {
            output( os, *child, s );
        }
        found = true;
    }
    for( boost::property_tree::ptree::const_iterator j = it->second.begin(); j != it->second.end(); ++j ) { traverse_( os, ptree, j, path ); }
    if( !( it->first.empty() ) ) { path = path.head(); }
    return found; // todo! aweful!
}

void match_regex_( std::ostream& os, const boost::property_tree::ptree& ptree )
{
    bool found = false;
    for( boost::property_tree::ptree::const_iterator i = ptree.begin(); i != ptree.end(); ++i )
    {
        comma::xpath path;
        found = traverse_( os, ptree, i, path ) || found;
    }
    if( found && output_last_line_break ) { os << std::endl; }
}

static bool is_regex_(const std::string& s)
{
    std::string regex_characters = regex_characters_;
    if (option_regex) { regex_characters += "[]"; }
    for( unsigned int k = 0; k < regex_characters.size(); ++k )
    {
        if( s.find_first_of( regex_characters[k] ) != std::string::npos ) { return true; }
    }
    return false;
}

int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av, usage );
        path_strings = options.unnamed( "--linewise,-l,--minify,--output-path,--use-buffer,--regex,--quote-numbers,--quote,--unquote-numbers,--unquote", "-.*" );
        if( path_strings.empty() ) { std::cerr << std::endl << "name-value-get: xpath missing" << std::endl; usage(); }
        path_regex.resize( path_strings.size() );
        paths.resize( path_strings.size() );
        bool has_regex = false;
        option_regex = options.exists( "--regex" );
        for( std::size_t i = 0; i < path_strings.size(); ++i )
        {
            if( is_regex_( path_strings[i] ) ) { path_regex[i] = boost::regex( path_strings[i], boost::regex::extended ); has_regex = true; }
            else {  paths[i] = boost::property_tree::ptree::path_type( path_strings[i], '/' ); }
        }
        boost::optional< std::string > from = options.optional< std::string >( "--from" );
        std::string to = options.value< std::string >( "--to", "path-value" );
        equal_sign = options.value( "--equal-sign,-e", '=' );
        linewise = options.exists( "--linewise,-l" );
        minify = options.exists( "--minify" );
        options.assert_mutually_exclusive( "--unquote-numbers,--unquote", "--quote-numbers,--quote" );
        unquote_numbers = options.exists( "--unquote-numbers,--unquote" );
        quote_numbers = options.exists( "--quote-numbers,--quote" ) && !unquote_numbers; // todo: quick and dirty, combine logic, it sucks now that there is different logic for json and everything else
        if ( options.exists( "--take-last" ) ) check_type = comma::property_tree::path_value::take_last;
        if ( options.exists( "--verify-unique,--unique-input" ) ) check_type = comma::property_tree::path_value::unique_input;
        boost::optional< char > delimiter = options.optional< char >( "--delimiter,-d" );
        path_value_delimiter = delimiter ? *delimiter : ( linewise ? ',' : '\n' );
        output_path = options.exists( "--output-path" );
        if( output_path && to != "path-value" ) { std::cerr << "name-value-get: --output-path is compatible only with --to=path-value; got: --to=" << to << std::endl; return 1; }
        if( from )
        {
            if( *from == "ini" ) { input = &traits< ini >::input; }
            else if( *from == "info" ) { input = &traits< info >::input; }
            else if( *from == "json" ) { input = &traits< json >::input; }
            else if( *from == "xml" ) { input = &traits< xml >::input; }
            else if( *from == "path-value" ) { input = &traits< path_value >::input; } // super-quick and dirty!
            else { std::cerr << "name-value-get: expected --from format to be ini, info, json, xml, or path-value, got " << *from << std::endl; return 1; }
        }
        else
        {
            if( linewise ) {  std::cerr << "name-value-get: if --linewise is present, --from must be given" << std::endl; return 1; }
            input = &traits< void_t >::input;
        }
        if( to == "ini" ) { output = &traits< ini >::output; }
        else if( to == "info" ) { output = &traits< info >::output; }
        else if( to == "json" ) { output = &traits< json >::output; }
        else if( to == "xml" ) { output = &traits< xml >::output; }
        else if( to == "path-value" ) { output = &traits< path_value >::output; output_last_line_break = true; }
        else { std::cerr << "name-value-get: expected --to format to be ini, info, json, xml, or path-value, got " << to << std::endl; return 1; }
        indices_mode = options.exists( "--no-brackets" ) ? comma::property_tree::without_brackets : comma::property_tree::with_brackets;
        if( linewise )
        {
            while( std::cout.good() )
            {
                std::string line;
                std::getline( std::cin, line );
                if( !std::cin.good() || std::cin.eof() ) { break; }
                std::istringstream iss( line );
                boost::property_tree::ptree ptree;
                input( iss, ptree );
                std::ostringstream oss;
                if( has_regex ) { match_regex_( oss, ptree ); } else { match_( oss, ptree ); }
                std::string s = oss.str();
                if( s.empty() ) { continue; }
                bool escaped = false;
                bool quoted = false;
                for( std::size_t i = 0; i < s.size(); ++i ) // quick and dirty
                {
                    if( escaped ) { escaped = false; continue; }
                    switch( s[i] )
                    {
                        case '\r': if( !quoted ) { s[i] = ' '; } break;
                        case '\\': escaped = true; break;
                        case '"' : quoted = !quoted; break;
                        case '\n': if( !quoted ) { s[i] = ' '; } break;
                    }
                }
                std::cout << s << std::endl;
            }
        }
        else
        {
            boost::property_tree::ptree ptree;
            input( std::cin, ptree );
            if( has_regex ) { match_regex_( std::cout, ptree ); } else { match_( std::cout, ptree ); }
        }
        return 0;
    }
    catch( boost::property_tree::ptree_bad_data& ex ) { std::cerr << "name-value-convert: bad data: " << ex.what() << std::endl; }
    catch( boost::property_tree::ptree_bad_path& ex ) { std::cerr << "name-value-convert: bad path: " << ex.what() << std::endl; }
    catch( boost::property_tree::ptree_error& ex ) { boost::regex e( "<unspecified file>" ); std::cerr << "name-value-convert: parsing error: " << boost::regex_replace( std::string( ex.what() ), e, "line" ) << std::endl; }
    catch( std::exception& ex ) { std::cerr << std::endl << "name-value-get: " << ex.what() << std::endl << std::endl; }
    catch( ... ) { std::cerr << std::endl << "name-value-get: unknown exception" << std::endl << std::endl; }
    return 1;
}
