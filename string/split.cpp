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
#include "split.h"

namespace comma {

bool string::is_one_of( char c, const char * characters )
{
    for( const char * s = characters; 0 != *s; ++s ) { if( c == *s ) return true; }
    return false;
}

std::vector< std::string > split( const std::string & s, const char * separators, bool empty_if_empty_input )
{
    std::vector< std::string > v;
    if( empty_if_empty_input && s.empty() ) { return v; }
    const char* begin( &s[0] );
    const char* end( begin + s.length() );
    v.push_back( std::string() );
    for( const char* p = begin; p < end; ++p )
    {
        if( string::is_one_of( *p, separators ) ) { v.push_back( std::string() ); } else { v.back() += *p; }
    }
    return v;
}

std::vector< std::string > split( const std::string & s, char separator, bool empty_if_empty_input )
{
    const char separators[] = { separator, 0 };
    return split( s, separators, empty_if_empty_input );
}

std::vector< std::string > split_escaped( const std::string & s, const char * separators, const char * quotes, char escape )
{
    std::vector< std::string > v;
    const char* begin( &s[0] );
    const char* const end( begin + s.length() );
    boost::optional< char > quoted;
    v.push_back( std::string() );
    for( const char* p = begin; p < end; ++p )
    {
        if( escape == *p )
        {
            ++p;
            if( end == p ) { v.back() += escape; break; }
            if( ! ( escape == *p || string::is_one_of( *p, separators ) || string::is_one_of( *p, quotes ) ) ) v.back() += escape;
            v.back() += *p;
        }
        else if( quoted == *p )
        {
            quoted = boost::none;
        }
        else if( ! quoted && string::is_one_of( *p, quotes ) )
        {
            quoted = *p;
        }
        else if( ! quoted && string::is_one_of( *p, separators ) )
        {
            v.push_back( std::string() );
        }
        else
        {
            v.back() += *p;
        }
    }
    if( quoted ) { COMMA_THROW( comma::exception, "quote not closed before end of string: " << s ); }
    return v;
}

std::vector< std::string > split_escaped( const std::string & s, char separator, const char * quotes, char escape )
{
    const char separators[] = { separator, 0 };
    return split_escaped( s, separators, quotes, escape );
}

std::vector< std::string > split_bracketed( const std::string& s, const char* separators, char lbracket, char rbracket )
{
    std::vector< std::string > v;
    const char* begin( &s[0] );
    const char* const end( begin + s.length() );
    unsigned int depth = 0;
    v.push_back( std::string() );
    for( const char* p = begin; p < end; ++p )
    {
        if( lbracket == *p )
        {
            ++depth;
            v.back() += *p;
        }
        else if( rbracket == *p )
        {
            if( depth > 0 ) { --depth; }
            v.back() += *p;
        }
        else if( depth == 0 && string::is_one_of( *p, separators ) )
        {
            v.push_back( std::string() );
        }
        else
        {
            v.back() += *p;
        }
    }
    return v;
}

std::vector< std::string > split_bracketed( const std::string& s, char separator, char lbracket, char rbracket )
{
    const char separators[] = { separator, 0 };
    return split_bracketed( s, separators, lbracket, rbracket );
}


} // namespace comma {
