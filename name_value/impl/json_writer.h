// This file is part of comma, a generic and flexible library
// Copyright (c) 2011 The University of Sydney

/// @author vsevolod vlaskine

#pragma once

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <boost/property_tree/json_parser.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include "../../base/exception.h"

namespace comma { namespace name_value { namespace impl {

template< typename C > inline void json_remove_quotes( std::basic_string< C >& json_text, bool pretty = true ) // assuming valid json
{
    using string_type = std::basic_string< C >;
    static string_type const true_str( std::initializer_list< C >{ 't', 'r', 'u', 'e' } );
    static string_type const false_str( std::initializer_list< C >{ 'f', 'a', 'l', 's', 'e' } );
    static boost::regex number_like_string( "^0[0-9][0-9]*$" );
    auto source = json_text.begin();
    auto target = json_text.cbegin();
    while( target != json_text.cend() )
    {
        auto value_begin = std::find( target, json_text.cend(), '"' );
        while( target != value_begin ) { *source++ = *target++; }
        if( json_text.cend() == value_begin ) { break; }
        auto value_end = value_begin;
        //do { value_end = std::find( value_end + 1, json_text.cend(), '"' ); } while( '\\' == *( value_end - 1 ) );
        while( true )
        {
            value_end = std::find( value_end + 1, json_text.cend(), '"' );
            unsigned int backslash_count = 0;
            for( auto i = value_end - 1; *i == '\\'; ++backslash_count, --i );
            if( backslash_count % 2 == 0 ) { break; } // hyper quick and dirty fix, sigh
        }
        auto next_token = std::find_if_not( value_end + 1, json_text.cend(), []( C ch ) { return ' ' == ch || '\t' == ch || '\n' == ch; } );
        bool quoted = true;
        if( ':' != *next_token )
        {
            auto value = std::string( value_begin + 1, value_end );
            for( char& c: value ) { c = std::tolower( c ); } // quick and dirty, watch performance
            if( true_str == value || false_str == value ) { quoted = false; }
            else if( value == "nan" || value == "inf" ) { quoted = true; } // lexical cast thinks it's a number
            else if( !boost::regex_match( value, number_like_string ) ) { try { boost::lexical_cast< double >( value ); quoted = false; } catch ( ... ) {} } // todo? try to avoid lexical_cast+exception to improve performace?
        }
        if( !quoted ) { value_begin++; }
        while( value_begin != value_end ) { *source++ = *value_begin++; }
        if( !quoted ) { value_end++; } 
        while( value_end != next_token ) { *source++ = *value_end++; }
        *source++ = *next_token++;
        target = next_token;
    }
    json_text.erase( pretty ? source : ( source - 1 ), json_text.end() );
}

template< class PTree > void write_json( std::basic_ostream< typename PTree::key_type::value_type > &stream, const PTree &ptree, bool const pretty = true, bool unquote_numbers = true )
{
    std::basic_ostringstream< typename PTree::key_type::value_type > oss;
    boost::property_tree::write_json( oss, ptree, pretty );
    if( unquote_numbers )
    {
        std::string s = oss.str();
        json_remove_quotes( s, pretty );
        stream << s;
    }
    else
    {
        if( pretty ) { stream << oss.str(); }
        else { std::string s = oss.str(); stream << s.substr( 0, s.size() - 1 ); } // unfortunately, boost adds trailing end of line, which we don't want in minified mode
    }
    stream << std::flush;
}
 
} } }
