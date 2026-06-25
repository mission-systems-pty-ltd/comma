// Copyright (c) 2011 The University of Sydney

#include <sstream>
#include "parser.h"

namespace comma { namespace name_value {

std::string parser::mangled( const std::string& line, const std::string& prefix, char delimiter )
{
    const auto& s = comma::split( line, delimiter, true );
    if( s.empty() ) { return line; }
    std::string p = prefix.empty() ? s[0] : prefix;
    std::string r;
    std::string d;
    for( unsigned int i = prefix.empty() ? 1 : 0; i < s.size(); ++i ) { r += d + p + '/' + s[i]; d = delimiter; }
    return r;
}

std::string parser::unaliased( const std::string& line, const std::unordered_map< std::string, std::string >& aliases, char delimiter, char value_delimiter )
{
    if( aliases.empty() ) { return line; }
    const auto& s = comma::split( line, delimiter, true );
    std::ostringstream oss;
    std::string d;
    for( const auto& p: s )
    {
        const auto& t = comma::split_head( p, 2, value_delimiter );
        auto i = aliases.find( t[0] );
        oss << d << ( i == aliases.end() ? p : ( i->second + ( t.size() > 1 ? value_delimiter + t[1] : std::string() ) ) ); 
        d = delimiter;
    }
    return oss.str();
}

} } // namespace comma { namespace name_value {
