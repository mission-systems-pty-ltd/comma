// Copyright (c) 2011 The University of Sydney

/// @author vsevolod vlaskine
/// @author mathew hounsell

#include <boost/optional.hpp>
#include "../base/exception.h"
#include "split.h"

namespace comma {

bool string::is_one_of( char c, const char * characters )
{
    for( const char * s = characters; 0 != *s; ++s ) { if( c == *s ) return true; }
    return false;
}

static std::vector< std::string > split_impl( const std::string & s, const char * separators, bool empty_if_empty_input, unsigned int size, bool head )
{
    std::vector< std::string > v;
    if( empty_if_empty_input && s.empty() ) { return v; }
    const char* begin( &s[0] );
    const char* end( begin + s.length() );
    v.push_back( std::string() );
    for( const char* p = begin; p < end; ++p )
    {
        if( string::is_one_of( *p, separators ) )
        {
            v.push_back( std::string() );
        }
        else
        {
            v.back() += *p;
        }
    }
    if( size == 0 || v.size() <= size ) { return v; }
    std::vector< std::string > r( size );
    if( head ) // quick and dirty for now
    {
        unsigned int sum{0};
        for( unsigned int i = 0; i < size; ++i ) { r[i] = v[i]; sum += v[i].size() + 1; }
        r.back() += std::string( s.substr( sum - 1 ) );
    }
    else
    {
        unsigned int sum{0}, k{0};
        for( unsigned int i = 0; i < v.size() - size + 1; ++i ) { sum += v[i].size() + 1; ++k; }
        r[0] = s.substr( 0, sum - 1 );
        for( unsigned int i = 1; i < size; ++i ) { r[i] = v[k + i - 1]; }
    }
    return r;
}

std::vector< std::string > split( const std::string & s, const char * separators, bool empty_if_empty_input )
{
    return split_impl( s, separators, empty_if_empty_input, 0, true ); 
}

std::vector< std::string > split( const std::string & s, char separator, bool empty_if_empty_input )
{
    const char separators[] = { separator, 0 };
    return split( s, separators, empty_if_empty_input );
}

std::vector< std::string > split_head( const std::string& s, unsigned int size, const char* separators, bool empty_if_empty_input )
{
    return split_impl( s, separators, empty_if_empty_input, size, true ); 
}

std::vector< std::string > split_head( const std::string& s, unsigned int size, char separator, bool empty_if_empty_input )
{
    const char separators[] = { separator, 0 };
    return split_head( s, size, separators, empty_if_empty_input );
}

std::vector< std::string > split_tail( const std::string& s, unsigned int size, const char* separators, bool empty_if_empty_input )
{
    return split_impl( s, separators, empty_if_empty_input, size, false ); 
}

std::vector< std::string > split_tail( const std::string& s, unsigned int size, char separator, bool empty_if_empty_input )
{
    const char separators[] = { separator, 0 };
    return split_tail( s, size, separators, empty_if_empty_input );
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
