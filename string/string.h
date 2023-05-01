// Copyright (c) 2011 The University of Sydney

/// @author vsevolod vlaskine

#pragma once

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

// escape given character and escape characters by preceding them with escape charcter.
std::string escape( const std::string & s, char character = '\'', char esc = '\\' );
// escape any of the given characters and escape character by preceding them with escape character
std::string escape( const std::string & s, const char* characters, char esc = '\\' );

// escape given character and escape characters by preceding them with escape charcter.
std::string unescape( const std::string & s, char character = '\'', char esc = '\\' );
// escape any of the given characters and escape character by preceding them with escape character
std::string unescape( const std::string & s, const char* characters, char esc = '\\' );

/// return common initial part of two strings
/// e.g. for abc and abd return ab; for abc and def return empty string
std::string common_front( const std::string& s, const std::string& t );

/// return common initial part of two strings as xpaths with a delimiter (probably should be in xpath)
/// e.g. for abc and abd return ab; for abc and def return empty string
std::string common_front( const std::string& s, const std::string& t, char delimiter ); 

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
    oss << a[0];
    for( std::size_t i = 1; i < size; ++i ) { oss << delimiter << a[i]; }
    return oss.str();
}

template < typename It >
inline std::string join( It begin, It end, char delimiter )
{
    if( begin == end ) { return ""; }
    std::ostringstream oss;
    oss << *begin;
    for( It i = ++begin; i != end; ++i ) { oss << delimiter << *i; }
    return oss.str();
}

} // namespace comma {
