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

#ifndef COMMA_STRING_STRING_H_
#define COMMA_STRING_STRING_H_

#include <stdlib.h>
#include <sstream>
#include <string>
#include <vector>
#include "../string/split.h"

namespace comma {

/// strip given characters from the beginning and end
std::string strip( const std::string& s, const char* characters = " \t\r\n" );

/// strip given character from the beginning and end
std::string strip( const std::string& s, char character );

// Escape given character and escape characters by preceding them with escape charcter.
std::string escape( const std::string & s, char character = '\'', char esc = '\\' );
// Escape any of the given characters and escape character by preceding them with escape character
std::string escape( const std::string & s, const char* characters, char esc = '\\' );

// Escape given character and escape characters by preceding them with escape charcter.
std::string unescape( const std::string & s, char character = '\'', char esc = '\\' );
// Escape any of the given characters and escape character by preceding them with escape character
std::string unescape( const std::string & s, const char* characters, char esc = '\\' );

/// join array elements into a string with given delimiter
template < typename A >
std::string join( const A& a, std::size_t size, char delimiter );

/// join array elements into a string with given delimiter
template < typename A >
inline std::string join( const A& a, char delimiter ) { return join( a, a.size(), delimiter ); }

template < typename A >
inline std::string join( const A& a, std::size_t size, char delimiter )
{
    if( size == 0 ) { return ""; }
    std::ostringstream oss;
    oss << a.front();
    typename A::const_iterator current = a.begin();
    while (++current != a.end() ) { oss << delimiter << *current; }
    return oss.str();
}

template < typename It >
inline std::string join( It begin, It end, char delimiter )
{
    if( begin == end ) { return ""; }
    std::ostringstream oss;
    oss << *begin;
    for( It i = begin + 1; i != end; ++i ) { oss << delimiter << *i; }
    return oss.str();
}

} // namespace comma {

#endif // COMMA_STRING_STRING_H_
