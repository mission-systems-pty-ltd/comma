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
/// @author mathew hounsell

#include <boost/optional.hpp>

// Don't use <> foc comma as that requires the code to be installed first.
#include "../base/exception.h"
#include "string.h"

#include <iostream>

namespace comma {

std::string strip( const std::string& s, char character )
{
    char characters[] = { character, 0 };
    return strip( s, characters );
}

std::string strip( const std::string& s, const char* characters )
{
    if( s.empty() ) return s;
    
    const std::string::size_type begin = s.find_first_not_of( characters );
    if( std::string::npos == begin ) return std::string();

    const std::string::size_type end = s.find_last_not_of( characters );
    // end can't be npos
    return s.substr( begin, end + 1 - begin );
}

std::string escape( const std::string & s, const char * characters, char esc )
{
    std::string v;
    const char* begin( &s[0] );
    const char* const end( begin + s.length() );
    for( const char* p = begin; p < end; ++p )
    {        
        if( esc == *p || string::is_one_of( *p, characters ) )
            v.push_back( esc );
        v.push_back( *p );
    }
    return v;
}

std::string escape( const std::string & s, char character, char esc )
{
    char characters[] = { character, 0 };
    return escape(s, characters, esc );
}

std::string unescape( const std::string & s, const char * characters, char esc )
{
    std::string v;
    const char* begin( &s[0] );
    const char* const end( begin + s.length() );
    for( const char* p = begin; p < end; ++p )
    {
        if( esc == *p )
        {
            const char* const next = p + 1;
            if( next == end )
            {
                v.push_back( esc );
                return v;
            }
            if ( *next != esc && ! string::is_one_of( *next, characters ) ) v.push_back( esc );
            ++p;
        }
        v.push_back( *p );
    }
    return v;
}

std::string unescape( const std::string & s, char character, char esc )
{
    char characters[] = { character, 0 };
    return unescape(s, characters, esc );
}

// for reference see split
std::string unescape_and_unquote( const std::string & s, char esc, const char* quotes )
{
    std::string v;
    const char* begin( &s[0] );
    const char* const end( begin + s.length() );
    boost::optional<char> quoted = boost::make_optional< char >( false, 0 );
    for( const char* p = begin; p < end; ++p )
    {
        if( esc == *p )
        {
            ++p;
            if( end == p ) { v.push_back( esc ); break; }
            if( ! ( esc == *p || string::is_one_of( *p, quotes ) ) ) v.push_back( esc );
            v.push_back( *p );
        }
        else if( quoted == *p )
        {
            quoted = boost::none;
        }
        else if( ! quoted && string::is_one_of( *p, quotes ) )
        {
            quoted = *p;
        }
        else
        {
            v.push_back( *p );
        }
    }
    if( quoted ) COMMA_THROW( comma::exception, "comma::unescape - quote not closed before end of string" );
    return v;
}

std::string common_front( const std::string& s, const std::string& t )
{
    std::string::size_type i = 0;
    for( ; i < s.size() && i < t.size() && s[i] == t[i]; ++i );
    return s.substr( 0, i );
}

std::string common_front( const std::string& s, const std::string& t, char delimiter )
{
    bool s_abs = !s.empty() && s[0] == delimiter;
    bool t_abs = !t.empty() && t[0] == delimiter;
    if( s_abs != t_abs ) { COMMA_THROW( comma::exception, "expected both paths absolute or both relative; got '" << s << "' and '" << t << "'" ); }
    std::string::size_type i = 0;
    std::string::size_type d = 0;
    for( ; i < s.size() && i < t.size() && s[i] == t[i]; ++i );
    if( i < s.size() && s[i] != delimiter )
    {
        i = s.find_last_of( delimiter, i );
        if( i == 0 ) { i = 1; } // root only
    }
    else if( i < t.size() && t[i] != delimiter )
    {
        i = t.find_last_of( delimiter, i );
        if( i == 0 ) { i = 1; } // root only
    }
    if( i == std::string::npos ) { i = 0; }
    else if( i == s.size() && s.size() > 1 && s.back() == delimiter ) { --i; }
    return s.substr( 0, i );
}


} // namespace comma {
