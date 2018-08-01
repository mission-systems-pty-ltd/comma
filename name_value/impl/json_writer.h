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

/// @author vsevolod vlaskine

#ifndef COMMA_NAME_VALUE_JSON_WRITER_H_
#define COMMA_NAME_VALUE_JSON_WRITER_H_

#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <boost/property_tree/json_parser.hpp>
#include <boost/lexical_cast.hpp>
#include "../../base/exception.h"


namespace comma { namespace name_value { namespace impl {

// NOTE: assume valid json.
template< typename C > void json_remove_quotes( std::basic_string< C >& json_text )
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
        do { value_end = std::find( value_end + 1, json_text.cend(), '"' ); } while( '\\' == *( value_end - 1 ) );
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
                    case ' ': case '\t': case '\n':
                        skip=true;
                        // todo
                        break;
                    case ':':
                        break;
                    default:
                        try { boost::lexical_cast< double >( value ); quoted = false; } catch ( ... ) {} // hyper quick and dirty for now
                        
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

#endif //COMMA_NAME_VALUE_JSON_WRITER_H_
