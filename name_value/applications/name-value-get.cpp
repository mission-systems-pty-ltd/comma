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
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/regex.hpp>
#include <comma/base/exception.h>
#include <comma/application/contact_info.h>
#include <comma/application/command_line_options.h>
#include <comma/application/signal_flag.h>
#include <comma/name_value/ptree.h>
#include <comma/xpath/xpath.h>

static void usage()
{
    std::cerr << std::endl;
    std::cerr << "take a stream of name-value style input on stdin," << std::endl;
    std::cerr << "output value at given path on stdout" << std::cerr;
    std::cerr << std::endl;
    std::cerr << "usage: cat data.xml | name-value-get <paths> [<options>]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "<paths>: x-path, e.g. \"command/type\" or posix regular expressions" << std::endl;
    std::cerr << std::endl;
    std::cerr << "data options" << std::endl;
    std::cerr << "    --from <what>" << std::endl;
    std::cerr << "    --to <what>" << std::endl;
    std::cerr << "      <what>" << std::endl;
    std::cerr << "        info: info data (see boost::property_tree)" << std::endl;
    std::cerr << "        ini: ini data" << std::endl;
    std::cerr << "        json: json data" << std::endl;
    std::cerr << "        name-value: name=value-style data; e.g. x={a=1,b=2},y=3" << std::endl;
    std::cerr << "        xml: xml data" << std::endl;
    std::cerr << "        default: --from: name-value; --to: same as --from" << std::endl;
    std::cerr << std::endl;
    std::cerr << "name/path-value options:" << std::endl;
    std::cerr << "    --equal-sign,-e=<equal sign>: default '='" << std::endl;
    std::cerr << "    --delimiter,-d=<delimiter>: default ','" << std::endl;
    std::cerr << "    --output-path: if path-value, output path (for regex)" << std::endl;
    std::cerr << "    --show-path-indices,--indices: show indices for array items e.g. y[0]/x/z[1]=\"a\"" << std::endl;
    std::cerr << "    --no-brackets: use with --show-path-indices - above, show indices as path elements e.g. y/0/x/z/1=\"a\"" << std::endl;
    std::cerr << std::endl;
    std::cerr << "path-value options:" << std::endl;
    std::cerr << "    --take-last: if paths are repeated, take last path=value" << std::endl;
    std::cerr << "    --verify-unique,--unique-input: ensure that all input paths are unique (takes precedence over --take-last)" << std::endl;
    std::cerr << "warning: if paths are repeated, output value selected from these inputs in not deterministic" << std::endl;
    std::cerr << std::endl;
    std::cerr << "data flow options:" << std::endl;
    std::cerr << "    --linewise,-l: if present, treat each input line as a record" << std::endl;
    std::cerr << "                   if absent, treat all of the input as one record" << std::endl;
    std::cerr << std::endl;
    std::cerr << comma::contact_info << std::endl;
    std::cerr << std::endl;
    exit( 1 );
}

static char equal_sign;
static char delimiter;
static bool linewise;
static bool output_path;
typedef comma::property_tree::path_mode path_mode;
static path_mode indices_mode = comma::property_tree::disabled;
static comma::property_tree::check_repeated_paths check_type( comma::property_tree::no_check );

enum Types { ini, info, json, xml, name_value, path_value };

template < Types Type > struct traits {};

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
    static void output( std::ostream& os, const boost::property_tree::ptree& ptree, const std::string& ) { boost::property_tree::write_json( os, ptree ); }
};

template <> struct traits< xml >
{
    static void input( std::istream& is, boost::property_tree::ptree& ptree ) { boost::property_tree::read_xml( is, ptree ); }
    static void output( std::ostream& os, const boost::property_tree::ptree& ptree, const std::string& ) { boost::property_tree::write_xml( os, ptree ); }
};

template <> struct traits< name_value >
{
    // todo: handle indented input (quick and dirty: use exceptions)
    static void input( std::istream& is, boost::property_tree::ptree& ptree ) { comma::property_tree::from_name_value( is, ptree, equal_sign, delimiter ); }
    static void output( std::ostream& os, const boost::property_tree::ptree& ptree, const std::string& ) { comma::property_tree::to_name_value( os, ptree, !linewise, equal_sign, delimiter ); }
};

template <> struct traits< path_value > // quick and dirty
{
    static void input( std::istream& is, boost::property_tree::ptree& ptree )
    {
        std::string s;
        if( linewise )
        {
            std::getline( is, s );
        }
        else
        {
            while( is.good() && !is.eof() ) // quick and dirty: read to the end of file
            {
                std::string t;
                std::getline( is, t );
                std::string::size_type pos = t.find_first_not_of( ' ' );
                if( pos == std::string::npos || t[pos] == '#' ) { continue; }
                s += t + delimiter;
            }
        }
        ptree = comma::property_tree::from_path_value_string( s, equal_sign, delimiter, check_type );
    }
    static void output( std::ostream& os, const boost::property_tree::ptree& ptree, const std::string& path )
    { 
        static bool first = true; // todo: will not work linewise, fix
        if( !first ) { std::cout << delimiter; }
        first = false;
        comma::property_tree::to_path_value( os, ptree, indices_mode, equal_sign, delimiter, output_path ? path : std::string() );
    }
};

static std::vector< std::string > path_strings;
static std::vector< boost::property_tree::ptree::path_type > paths;
static std::vector< boost::optional< boost::regex > > path_regex;
static void ( * input )( std::istream& is, boost::property_tree::ptree& ptree );
static void ( * output )( std::ostream& is, const boost::property_tree::ptree& ptree, const std::string& );

void match_( std::ostream& os, const boost::property_tree::ptree& ptree )
{
    static const boost::property_tree::ptree::path_type empty;
    for( std::size_t i = 0; i < paths.size(); ++i )
    {
        boost::optional< const boost::property_tree::ptree& > child = ptree.get_child_optional( paths[i] );
        if( !child ) { continue; }
        boost::optional< std::string > value = child->get_optional< std::string >( empty );
        if( value && !value->empty() )
        { 
            if( output_path ) { os << path_strings[i] << equal_sign; }
            os << *value << std::endl;
        }
        else
        { 
            output( os, *child, path_strings[i] );
        }
    }
}

static void traverse_( std::ostream& os, const boost::property_tree::ptree& ptree, boost::property_tree::ptree::const_iterator it, comma::xpath& path )
{
    static const boost::property_tree::ptree::path_type empty;
    path /= it->first;
    const std::string& s = path.to_string( '/' ); // quick and dirty
    for( std::size_t i = 0; i < paths.size(); ++i ) // todo: quick and dirty: can prune much earlier, i guess...
    {
        if( path_regex[i] ) { if( !boost::regex_match( s, *path_regex[i] ) ) { continue; } }
        else if( s != path_strings[i] ) { continue; }
        boost::optional< const boost::property_tree::ptree& > child = ptree.get_child_optional( path.to_string( '.' ) ); // quick and dirty, watch performance
        if( !child ) { continue; }
        boost::optional< std::string > value = child->get_optional< std::string >( empty );
        if( value && !value->empty() )
        { 
            if( output_path ) { os << s << equal_sign; }
            os << *value << std::endl;
        }
        else
        {
            output( os, *child, s );
        }
    }
    for( boost::property_tree::ptree::const_iterator j = it->second.begin(); j != it->second.end(); ++j )
    {
        traverse_( os, ptree, j, path );
    }
    if( !(it->first.empty()) ) { path = path.head(); }
}

void match_regex_( std::ostream& os, const boost::property_tree::ptree& ptree )
{
    for( boost::property_tree::ptree::const_iterator i = ptree.begin(); i != ptree.end(); ++i )
    {
        comma::xpath path;
        traverse_( os, ptree, i, path );
    }
}

int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av );
        if( options.exists( "--help,-h" ) ) { usage(); }
        path_strings = options.unnamed( "--linewise,-l,--output-path", "--from,--to,--equal-sign,-e,--delimiter,-d" );
        if( path_strings.empty() ) { std::cerr << std::endl << "name-value-get: xpath missing" << std::endl; usage(); }
        path_regex.resize( path_strings.size() );
        paths.resize( path_strings.size() );
        bool has_regex = false;
        for( std::size_t i = 0; i < path_strings.size(); ++i )
        {
            // todo: add vector support
            static const std::string regex_characters = ".[]{}()\\*+?|^$";
            for( unsigned int k = 0; k < regex_characters.size(); ++k )
            { 
                if( path_strings[i].find_first_of( regex_characters[k] ) != std::string::npos )
                {
                    path_regex[i] = boost::regex( path_strings[i], boost::regex::extended );
                    has_regex = true;
                    break;
                }
            }
            if( !path_regex[i] ) { paths[i] = boost::property_tree::ptree::path_type( path_strings[i], '/' ); }
        }
        std::string from = options.value< std::string >( "--from", "name-value" );
        std::string to = options.value< std::string >( "--to", "name-value" );
        equal_sign = options.value( "--equal-sign,-e", '=' );
        delimiter = options.value( "--delimiter,-d", from == "path-value" || to == "path-value" ? '\n' : ',' );
        linewise = options.exists( "--linewise,-l" );
        if ( options.exists( "--take-last" ) ) check_type = comma::property_tree::take_last;
        if ( options.exists( "--verify-unique,--unique-input" ) ) check_type = comma::property_tree::unique_input;
        output_path = options.exists( "--output-path" );
        if( from == "ini" ) { input = &traits< ini >::input; output = &traits< ini >::output; }
        else if( from == "info" ) { input = &traits< info >::input; output = &traits< info >::output; }
        else if( from == "json" ) { input = &traits< json >::input; output = &traits< json >::output; }
        else if( from == "xml" ) { input = &traits< xml >::input; output = &traits< xml >::output; }
        else if( from == "path-value" ) { input = &traits< path_value >::input; }
        else { input = &traits< name_value >::input; output = &traits< name_value >::output; }
        if( to == "ini" ) { output = &traits< ini >::output; }
        else if( to == "info" ) { output = &traits< info >::output; }
        else if( to == "json" ) { output = &traits< json >::output; }
        else if( to == "xml" ) { output = &traits< xml >::output; }
        else if( to == "path-value" ) { output = &traits< path_value >::output; }
        else if( to == "name-value" ) { output = &traits< name_value >::output; }
        if( options.exists( "--show-path-indices,--indices" ) ) 
        {
            if( options.exists( "--no-brackets" ) ) { indices_mode = comma::property_tree::without_brackets; }
            else { indices_mode = comma::property_tree::with_brackets; }
        }
        if( linewise )
        {
            comma::signal_flag is_shutdown;
            while( std::cout.good() )
            {
                std::string line;
                std::getline( std::cin, line );
                if( is_shutdown || !std::cin.good() || std::cin.eof() ) { break; }
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
    }
    catch( boost::property_tree::ptree_bad_data& ex )
    {
        std::cerr << "name-value-convert: bad data: " << ex.what() << std::endl;
    }
    catch( boost::property_tree::ptree_bad_path& ex )
    {
        std::cerr << "name-value-convert: bad path: " << ex.what() << std::endl;
    }
    catch( boost::property_tree::ptree_error& ex )
    {
        boost::regex e( "<unspecified file>" );
        std::cerr << "name-value-convert: parsing error: " << boost::regex_replace( std::string( ex.what() ), e, "line" ) << std::endl;
    }
    catch( std::exception& ex )
    {
        std::cerr << std::endl << "name-value-get: " << ex.what() << std::endl << std::cerr << std::endl;
        return 1;
    }
    catch( ... )
    {
        std::cerr << std::endl << "name-value-get: unknown exception" << std::endl << std::endl;
        return 1;
    }
    return 0;
}
