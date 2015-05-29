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

#include <comma/base/exception.h>
// Don't use the <> because that requires the code to be installed first.
#include "split.h"

namespace comma {

static bool is_one_of( const char c, const char * const separators )
{
    for( const char* s = separators; *s; ++s ) { if( c == *s ) { return true; } }
    return false;
}

std::vector< std::string > split( const std::string & s, const char * const separators )
{
    std::vector< std::string > v;
    const char* begin( &s[0] );
    const char* end( begin + s.length() );
    v.push_back( std::string() );
    for( const char* p = begin; p < end; ++p )
    {
        if( is_one_of( *p, separators ) )
            v.push_back( std::string() );
        else
            v.back() += *p;
    }
    return v;
}

std::vector< std::string > split( const std::string & s, const char separator )
{
    const char separators[] = { separator, 0 };
    return split( s, separators );
}

std::vector< std::string > split_escaped( const std::string & s, const char * const separators, const char escape, const char * const quotes )
{
    std::vector< std::string > v;
    const char* begin( &s[0] );
    const char* const end( begin + s.length() );
    bool prev_was_delimiter = true;
    bool quoted = false;
    v.push_back( std::string() );
    for( const char* p = begin; p < end; ++p )
    {
        if( escape == *p )
        {
            ++p;
            if( end == p ) COMMA_THROW( comma::exception, "comma::split_escaped - line can not end in an escape character" );
            v.back() += *p;
        }
        else if( is_one_of( *p, quotes ) )
        {
            if( ! quoted )
            {
                if( ! prev_was_delimiter ) COMMA_THROW( comma::exception, "comma::split_escaped - start quote must follow delimiter or be first character" );
            }
            else
            {
                const char next = ( end != p + 1 ? *( p + 1 ) : 0 );
                if( 0 != next && ! is_one_of( next, separators ) ) COMMA_THROW( comma::exception, "comma::split_escaped - end quote must precede a delimiter or be last character" );
            }
            quoted = ! quoted;
        }
        else if( ! quoted && is_one_of( *p, separators ) )
        {
            v.push_back( std::string() );
            prev_was_delimiter = true;
            continue;
        }
        else
        {
            v.back() += *p;
        }
        prev_was_delimiter = false;
    }
    if( quoted ) COMMA_THROW( comma::exception, "comma::split_escaped - quote not closed before end of string" );
    return v;
}

std::vector< std::string > split_escaped( const std::string & s, const char separator, const char escape, const char quote )
{
    const char separators[] = { separator, 0 };
    const char quotes[] = { quote, 0 };
    return split_escaped( s, separators, escape, quotes );
}

} // namespace comma {

// //     char const map_escape[256] = { 1, };
// //     map_escape['0'] = 0;
// //     map_escape['a'] = '\a';
// //     map_escape['b'] = '\b';
// //     map_escape['f'] = '\f';
// //     map_escape['n'] = '\n';
// //     map_escape['r'] = '\r';
// //     map_escape['t'] = '\t';
// //     map_escape['v'] = '\v';
// //     map_escape['?'] = '?';
// //     map_escape[escape] = escape;
// //     for( unsigned i = 0; 0 != separators[i]; ++i ) map_escape[i] = separators[i];
// //     for( unsigned i = 0; 0 != separators[i]; ++i ) map_escape[i] = quotes[i];
// //     
