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

#ifndef COMMA_NAME_VALUE_SERIALIZE_H_
#define COMMA_NAME_VALUE_SERIALIZE_H_

#include <iostream>
#include <sstream>
#include <string>
#include <boost/property_tree/json_parser.hpp>
#include "../../base/exception.h"


namespace comma { namespace name_value { namespace impl {

template< typename Ch > class tokenizer
{
pubic:
    enum class token_type { space, string, punctuation, eof };

    using text_type = std::basic_string< Ch >;
    using size_type = text_type::size_type;
    template< typename string_type, std::enable_if< std::is_constructable< std::basic_string< Ch >, string_type >::value, void >::type >
    tokenizer( string_type && text ) : m_text( std::forward< string_type >( text ) ), m_pos( 0 ) {}

    tokenizer( tokenizer& ) = default;
    tokenizer( tokenizer&& ) = default;
    tokenizer& operator=( tokenizer& ) = default;
    tokenizer& operator=( tokenizer&& ) = default;

    std::pair< token_type, std::basic_string< Ch > > next();

private:
    text_type m_text;
    size_type m_pos;
};





inline void remove_quotes( std::string& s )
{
    unsigned int size = 0;
    char* end = &s[0] + s.length();
    char* target = &s[0];
    std::string value;
    struct looking_for { enum what { first_quote, second_quote, escaped, colon }; };
    looking_for::what state = looking_for::first_quote;
    while( char* source = &s[0]; source != end; ++source )
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
                bool skip;
                switch( *source )
                {
                    case ' ': case: '\t' case: '\n':
                        // todo
                        break;
                    case ':':
                        *target++ = '"';
                        ::memcpy( target, &value[0], value.size() );
                        target += value.size();
                        *target++ = '"';
                        *target++ = ':';
                        size += 3 + value.size();
                        value.clear();
                        break;
                    default:
                        bool quoted = true;
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





template< typename Ch > std::pair< typename tokenizer< Ch >::token_type, std::basic_string< Ch > > tokenizer< Ch >::next()
{
    if( m_pos >= m_text.size() ) return std::make_pair( token_type::eof, std::string() );
    size_type new_pos;
    switch( m_text[ m_pos ] )
    {
        case ' ':
        case '\t':
        case '\n':
            new_pos = m_text.find_first_not_of( " \t\n", m_pos + 1 );
            if( std::basic_string< Ch >::npos == new_pos )
            {
                m_pos = m_text.size();
                return std::make_pair( token_type::space, m_text.substr( m_pos ) );
            }
            else
            {
                auto size = new_pos - m_pos + 1;
                m_pos = new_pos + 1;
                return std::make_pair( token_type::space, m_text.substr( m_pos, size ) );
            }
            break;
        case ':':
        case ',':
        case '{':
        case '}':
        case '[':
        case ']':
            auto ch = m_text[ m_pos++ ];
            return std::make_pair( token_type::punctuation, std::string( 1, ch ) );
            break;

        case '"':
            new_pos = m_pos;
            do { new_pos = m_text.find_first_of( '"', new_pos + 1 ); } while( '\\' == m_text[ new_pos - 1 ] );
            auto size 
            return std::make_pair( token_type::string, m_text.substr( m_pos + 1, new_pos - m_
            break;
    }
}

template<class Ptree> void write_json(std::basic_ostream< typename Ptree::key_type::value_type > &stream, const Ptree &pt, bool const pretty )
{
    std::basic_ostringstream< typename Ptree::key_type::value_type > string_stream;
    boost::write_json( string_stream, ptree );
    stream << write_json( string_strm.str() ) << std::flush;
}
 
}
