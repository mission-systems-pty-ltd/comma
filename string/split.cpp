// Copyright (c) 2011 The University of Sydney

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

std::vector< std::string > split_bracketed( const std::string& s, const char* separators, char lbracket, char rbracket, bool strip_brackets )
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
            if( strip_brackets && depth == 0 )
            {
                if( !v.back().empty() ) { COMMA_THROW( comma::exception, "asked to strip brackets; expected opening bracket immediately following separator, got'" << s << "'" ); }
            }
            else
            {
                v.back() += *p;
            }
            ++depth;
        }
        else if( rbracket == *p )
        {
            if( !strip_brackets || depth > 1 ) { v.back() += *p; }
            if( depth > 0 ) { --depth; }
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

std::vector< std::string > split_bracketed( const std::string& s, char separator, char lbracket, char rbracket, bool strip_brackets )
{
    const char separators[] = { separator, 0 };
    return split_bracketed( s, separators, lbracket, rbracket, strip_brackets );
}


} // namespace comma {
