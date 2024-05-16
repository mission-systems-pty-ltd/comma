// Copyright (c) 2011 The University of Sydney

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

} } // namespace comma { namespace name_value {
