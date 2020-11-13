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
#include "../../base/exception.h"

namespace comma { namespace name_value { namespace impl {

template< typename C > void json_remove_quotes( std::basic_string< C >& json_text ) // assuming valid json
{
    using string_type = std::basic_string< C >;
    string_type const true_str( std::initializer_list< C >{ 't', 'r', 'u', 'e' } );
    string_type const false_str( std::initializer_list< C >{ 'f', 'a', 'l', 's', 'e' } );
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
            auto i = value_end - 1;
            if( *i != '\\' || *( i - 1 ) == '\\' ) { break; } // hyper quick and dirty fix, sigh
        }
        auto next_token = std::find_if_not( value_end + 1, json_text.cend(), []( C ch ) { return ' ' == ch || '\t' == ch || '\n' == ch; } );
        bool quoted = true;
        if( ':' != *next_token )
        {
            auto const value = std::string( value_begin + 1, value_end );
            if( true_str == value || false_str == value ) { quoted = false; }
            else { try { boost::lexical_cast< double >( value ); quoted = false; } catch ( ... ) {} }
        }
        if( !quoted ) { value_begin++; }
        while( value_begin != value_end ) { *source++ = *value_begin++; }
        if( !quoted ) { value_end++; } 
        while( value_end != next_token ) { *source++ = *value_end++; }
        *source++ = *next_token++;
        target = next_token;
    }
    json_text.erase( source, json_text.cend() );
}

inline void remove_quotes( std::string& s )
{
    unsigned int size = 0;
    char* end = &s[0] + s.length();
    char* target = &s[0];
    std::string value;
    struct looking_for { enum what { first_quote, second_quote, escaped, colon }; };
    looking_for::what state = looking_for::first_quote;
    for( char* source = &s[0]; source != end; ++source )
    {
        switch( state )
        {
            case looking_for::first_quote:
                if( *source == '"' ) { state = looking_for::second_quote; } else { *target++ = *source; ++size; }
                break;
            case looking_for::second_quote:
                if( *source == '"' ) { state = looking_for::colon; }
                else { value += *source; if( *source == '\\' ) { state = looking_for::escaped; } }
                break;
            case looking_for::escaped:
                value += *source;
                state = looking_for::second_quote;
                break;
            case looking_for::colon:
            {
                bool quoted = true;
                bool skip = false;
                switch( *source )
                {
                    case ' ': case '\t': case '\n': skip=true; break;
                    case ':': break;
                    default: try { boost::lexical_cast< double >( value ); quoted = false; } catch ( ... ) {} // hyper quick and dirty for now
                }
                if( skip ) { break; }
                if( quoted ) { *target++ = '"'; ++size; }
                ::memcpy( target, &value[0], value.size() );
                target += value.size();
                size += value.size();
                if( quoted ) { *target++ = '"'; ++size; }
                *target++ = *source;
                ++size;
                value.clear();
                state = looking_for::first_quote;
                break;
            }
        }
    }
    s.resize( size );
}

template<class Ptree> void write_json(std::basic_ostream< typename Ptree::key_type::value_type > &stream, const Ptree &ptree, bool const pretty = true )
{
    std::basic_ostringstream< typename Ptree::key_type::value_type > string_stream;
    boost::property_tree::write_json( string_stream, ptree, pretty );
    auto json_text = string_stream.str();
    json_remove_quotes( json_text );
    stream << json_text << std::flush;
}
 
} } }
