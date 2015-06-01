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

namespace comma {

std::string strip( const std::string& s, char character )
{
    char characters[] = { character, 0 };
    return strip( s, characters );
}

std::string strip( const std::string& s, const char* characters )
{
    if( s.empty() ) { return s; }
    std::size_t begin = 0;
    while( begin < s.length() && is_one_of( s.at( begin ), characters ) ) { ++begin; }
    std::size_t end = s.length() - 1;
    while( end > begin && is_one_of( s.at( end ), characters ) ) { --end; }
    return s.substr( begin, end + 1 - begin );
}

std::string escape( const std::string & s, char esc, const char * characters )
{
    std::string v;
    const char* begin( &s[0] );
    const char* const end( begin + s.length() );
    for( const char* p = begin; p < end; ++p )
    {        
        if( esc == *p || is_one_of( *p, characters ) )
            v.push_back( esc );
        v.push_back( *p );
    }
    return v;
}

std::string escape( const std::string & s, char esc, char quote )
{
    char characters[] = { quote, 0 };
    return escape(s, esc, characters );
}

std::string escape( const std::string & s, char esc, char quote, char delimiter )
{
    char characters[] = { quote, delimiter, 0 };
    return escape(s, esc, characters );
}

std::string unescape( const std::string & s, char esc, const char* quotes )
{
    std::string v;
    const char* begin( &s[0] );
    const char* const end( begin + s.length() );
    boost::optional<char> quoted;
    for( const char* p = begin; p < end; ++p )
    {
        if( esc == *p )
        {
            ++p;
            if( end == p ) { v.push_back( esc ); break; }
            if( ! ( esc == *p || is_one_of( *p, quotes ) ) ) v.push_back( esc );
            v.push_back( *p );
        }
        else if( quoted == *p )
        {
            quoted = boost::none;
        }
        else if( ! quoted && is_one_of( *p, quotes ) )
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

std::string unescape( const std::string & s, char esc, char quote )
{
    char quotes[] = { quote, 0 };
    return unescape(s, esc, quotes );
}

} // namespace comma {
