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


/// @author cedric wohlleber
/// @author vsevolod vlaskine

#include <iostream>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/unordered_set.hpp>
#include <comma/base/exception.h>
#include <comma/base/types.h>
#include <comma/string/string.h>
#include <comma/xpath/xpath.h>
#include <comma/visiting/visit.h>
#include <comma/visiting/while.h>

#include <comma/name_value/ptree.h>

#define TEST_UNIQUE 1

namespace comma { namespace Impl {

static void ptree_to_name_value_string_impl( std::ostream& os, boost::property_tree::ptree::const_iterator i, bool is_begin, bool indented, unsigned int indent, char equal_sign, char delimiter )
{
    if( !is_begin && !( indented && ( delimiter == ' ' || delimiter == '\t' ) ) ) { os << delimiter; }
    if( indented ) { os << std::endl << std::string( indent, ' ' ); }
    os << i->first << equal_sign;
    if( i->second.begin() == i->second.end() )
    {
        //std::string value = i->second.get_value< std::string >();
        //bool quoted =    value.empty() // real quick and dirty
        //              || value.find_first_of( '\\' ) != std::string::npos
        //              || value.find_first_of( '"' ) != std::string::npos;
        //if( quoted ) { os << '"'; }
        //os << value;
        //if( quoted ) { os << '"'; }
        os << '"' << i->second.get_value< std::string >() << '"';
    }
    else
    {
        if( indented ) { os << std::endl << std::string( indent, ' ' ); }
        os << "{";
        for( boost::property_tree::ptree::const_iterator j = i->second.begin(); j != i->second.end(); ++j )
        {
            ptree_to_name_value_string_impl( os, j, j == i->second.begin(), indented, indent + 4, equal_sign, delimiter );
        }
        if( indented ) { os << std::endl << std::string( indent, ' ' ); }
        os << '}';
    }
}

static void ptree_output_value_( std::ostream& os, const std::string& value, bool is_begin, const xpath& path, char equal_sign, char delimiter, const std::string& root )
{
    if( !is_begin ) { os << delimiter; }
    if( root != "" ) { os << root << "/"; }
    os << path.to_string() << equal_sign << '"' << value << '"';
}

static void ptree_to_path_value_string_impl( std::ostream& os, boost::property_tree::ptree::const_iterator i, bool is_begin, xpath& path, xpath& display_path, 
                                                    property_tree::path_mode mode, char equal_sign, char delimiter, const std::string& root )
{
    if( i->second.begin() == i->second.end() )
    {
        ptree_output_value_( os, i->second.get_value< std::string >(), is_begin, display_path / i->first, equal_sign, delimiter, root );
    }
    else
    {
        path /= i->first;
        display_path /= i->first;
        boost::optional< std::string > v = i->second.get_value_optional< std::string >();
        if( v ) // quick and dirty
        {
            const std::string& stripped = comma::strip( *v );
            if( !stripped.empty() )  { ptree_output_value_( os, stripped, is_begin, display_path, equal_sign, delimiter, root );  }
        }
        
        comma::uint32 index=0;
        for( boost::property_tree::ptree::const_iterator j = i->second.begin(); j != i->second.end(); ++j )
        {
            // Test if it is json array data, if so all keys are empty. If so display indices in path if requested
            if( mode == property_tree::without_brackets && j->first.empty()  ) { display_path /= boost::lexical_cast< std::string >( index++ ); }
            else if( mode == property_tree::with_brackets && j->first.empty() ) { display_path.elements.back().index = index++; }
            ptree_to_path_value_string_impl( os, j, is_begin, path, display_path, mode, equal_sign, delimiter, root );
            if( mode == property_tree::without_brackets && j->first.empty() ) { display_path = display_path.head(); }
            is_begin = false;
        }
        if( !(i->first.empty()) ) { path = path.head(); display_path = display_path.head(); } // for json arrays, the keys are empty
    }
}

} } // namespace { namespace Impl {


namespace comma {

void property_tree::to_name_value( std::ostream& os, const boost::property_tree::ptree& ptree, bool indented, char equal_sign, char delimiter )
{
    for( boost::property_tree::ptree::const_iterator i = ptree.begin(); i != ptree.end(); ++i )
    {
        Impl::ptree_to_name_value_string_impl( os, i, i == ptree.begin(), indented, 0, equal_sign, delimiter );
    }
    if( indented ) { os << std::endl; } // quick and dirty
}

void property_tree::to_path_value( std::ostream& os, const boost::property_tree::ptree& ptree, path_mode mode, char equal_sign, char delimiter, const xpath& root )
{
    for( boost::property_tree::ptree::const_iterator i = ptree.begin(); i != ptree.end(); ++i )
    {
        xpath path;
        // display_path is the modified key path showing array indices, if array exists within e.g flight_levels[0]/levels[0]
        // But the actual path to the value is many empty keys under flight_levels and flight_levels/levels
        // Boost: "JSON arrays are mapped to nodes. Each element is a child node with an empty name. 
        //         If a node has both named and unnamed child nodes, it cannot be mapped to a JSON representation."
        // http://www.boost.org/doc/libs/1_41_0/doc/html/boost_propertytree/parsers.html#boost_propertytree.parsers.json_parser
        xpath display_path;
        Impl::ptree_to_path_value_string_impl( os, i, i == ptree.begin(), path, display_path, mode, equal_sign, delimiter, root.to_string() ); // quick and dirty
    }
}

void property_tree::from_name_value( std::istream& is, boost::property_tree::ptree& ptree, char equal_sign, char delimiter )
{
    // quick and dirty, worry about performance, once needed
    std::ostringstream oss;
    while( is.good() && !is.eof() )
    {
        std::string line;
        std::getline( is, line );
        if( !line.empty() && line.at( 0 ) != '#' ) { oss << line; } // quick and dirty: allow comment lines
    }
    ptree = from_name_value_string( oss.str(), equal_sign, delimiter );
}

void property_tree::from_path_value( std::istream& is, boost::property_tree::ptree& ptree, check_repeated_paths check_type, char equal_sign, char delimiter )
{
    std::string s;
    while( is.good() && !is.eof() ) // quick and dirty: read to the end of file
    {
        std::string line;
        std::getline( is, line );
        std::string::size_type pos = line.find_first_not_of( ' ' );
        if( pos == std::string::npos || line[pos] == '#' ) { continue; }
        s += line + delimiter;
    }
    ptree = comma::property_tree::from_path_value_string( s, equal_sign, delimiter, check_type );
}

std::string property_tree::to_name_value_string( const boost::property_tree::ptree& ptree, bool indented, char equal_sign, char delimiter )
{
    std::ostringstream oss;
    to_name_value( oss, ptree, indented, equal_sign, delimiter );
    return oss.str();
}

std::string property_tree::to_path_value_string( const boost::property_tree::ptree& ptree, property_tree::path_mode mode, char equal_sign, char delimiter )
{
    std::ostringstream oss;
    to_path_value( oss, ptree, mode, equal_sign, delimiter );
    return oss.str();
}

boost::property_tree::ptree property_tree::from_name_value_string( const std::string& s, char equal_sign, char delimiter )
{
    boost::property_tree::ptree ptree;
    bool escaped = false;
    bool quoted = false;
    std::ostringstream oss;
    for( std::size_t i = 0; i < s.length(); ++i )
    {
        char c = s[i];
        bool space = false;
        if( escaped )
        {
            escaped = false;
        }
        else
        {
            switch( c )
            {
                case '\\':
                    escaped = true;
                    break;
                case '"':
                    quoted = !quoted;
                    break;
                case '{':
                case '}':
                    space = !quoted;
                default:
                    if( quoted ) { break; }
                    if( c == equal_sign ) { c = ' '; }
                    else if( c == delimiter ) { c = ' '; }
                    break;
            }
        }
        if( space ) { oss << ' '; }
        oss << c;
        if( space ) { oss << ' '; }
    }
    std::istringstream iss( oss.str() );
    boost::property_tree::read_info( iss, ptree );
    return ptree;
}

boost::property_tree::ptree& property_tree::from_path_value_string( boost::property_tree::ptree& ptree, const std::string& s, char equal_sign, char delimiter, check_repeated_paths check_type )
{
    switch ( check_type ) {
        case comma::property_tree::take_last         : return Impl::from_path_value_string< comma::property_tree::take_last >::parse( ptree, s, equal_sign, delimiter );
        case comma::property_tree::unique_input      : return Impl::from_path_value_string< comma::property_tree::unique_input >::parse( ptree, s, equal_sign, delimiter );
        case comma::property_tree::no_overwrite      : return Impl::from_path_value_string< comma::property_tree::no_overwrite >::parse( ptree, s, equal_sign, delimiter );
        case comma::property_tree::no_check: default : return Impl::from_path_value_string< comma::property_tree::no_check >::parse( ptree, s, equal_sign, delimiter );
    }
    return ptree;
}

boost::property_tree::ptree property_tree::from_path_value_string( const std::string& s, char equal_sign, char delimiter, check_repeated_paths check_type )
{
    boost::property_tree::ptree ptree;
    from_path_value_string( ptree, s, equal_sign, delimiter, check_type );
    return ptree;
}

bool is_seekable( std::istream& stream ) { return stream.seekg( 0, std::ios::beg ); }

void property_tree::from_unknown( std::istream& stream, boost::property_tree::ptree& ptree, check_repeated_paths check_type, char equal_sign, char delimiter )
{
    if( is_seekable( stream ) ) 
    {
        from_unknown_seekable( stream, ptree, check_type, equal_sign, delimiter); 
    }
    else 
    {
        std::stringstream buffer;
        buffer << stream.rdbuf();
        from_unknown_seekable( buffer, ptree, check_type, equal_sign, delimiter);
    }
}

void property_tree::from_unknown_seekable( std::istream& stream, boost::property_tree::ptree& ptree, check_repeated_paths check_type, char equal_sign, char delimiter )
{
    if( !is_seekable( stream ) ) { COMMA_THROW( comma::exception, "input stream is not seekable" ); }
    try
    {
        stream.clear();
        stream.seekg( 0, std::ios::beg );
        boost::property_tree::read_json( stream, ptree );
        return;
    }
    catch( const boost::property_tree::ptree_error&  ex ) {}
    catch(...) { throw; }
    try
    {
        stream.clear();
        stream.seekg( 0, std::ios::beg );
        boost::property_tree::read_xml( stream, ptree );
        return;
    }
    catch( const boost::property_tree::ptree_error&  ex ) {}
    catch(...) { throw; }
    try
    {
        stream.clear();
        stream.seekg( 0, std::ios::beg );
        comma::property_tree::from_path_value( stream, ptree, check_type, equal_sign, delimiter );
        return;
    }
    catch( const boost::property_tree::ptree_error&  ex ) {}
    catch( const comma::exception&  ex ) {}
    catch(...) { throw; }
    // TODO: add try for ini format (currently the problem is that path-value treats ini sections and comments as valid entries; possible solution: make path-value parser stricter)
    COMMA_THROW( comma::exception, "failed to guess format" );
}

} // namespace comma {
