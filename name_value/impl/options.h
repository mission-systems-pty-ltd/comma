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


/// @author cedric wohlleber

#ifndef COMMA_APPLICATION_NAME_VALUE_OPTIONS_H
#define COMMA_APPLICATION_NAME_VALUE_OPTIONS_H

#include "../../base/exception.h"
#include "../../string/string.h"
#include "../../xpath/xpath.h"

namespace comma
{
namespace name_value
{
namespace impl
{

/// name_value options
struct options
{
    options( char delimiter = ';', char value_delimiter = '=', bool full_path_as_name = true, const char * quotes = "\'\"", char escape = '\\' );
    options( const std::string& fields, char delimiter = ';', char value_delimiter = '=', bool full_path_as_name = true, const char * quotes = "\'\"", char escape = '\\' );
    
    char m_delimiter;
    char m_value_delimiter;
    bool m_full_path_as_name;
    char m_escape;
    std::string m_quotes;
    std::vector< std::string > m_names; /// names for unnamed values
};


inline options::options( char delimiter, char value_delimiter, bool full_path_as_name, const char * quotes, char escape ):
    m_delimiter( delimiter ),
    m_value_delimiter( value_delimiter ),
    m_full_path_as_name( full_path_as_name ),
    m_escape( escape ),
    m_quotes( quotes )
{
}

inline options::options( const std::string& fields, char delimiter, char value_delimiter, bool full_path_as_name, const char * quotes, char escape ):
    m_delimiter( delimiter ),
    m_value_delimiter( value_delimiter ),
    m_full_path_as_name( full_path_as_name ),
    m_escape( escape ),
    m_quotes( quotes ),
    m_names( split( fields, ',' ) )
{
    if( fields.empty() ) { COMMA_THROW( comma::exception, "expected fields, got empty string" ); }
    if( full_path_as_name ) { return; }
    for( std::size_t i = 0; i < m_names.size(); ++i ) { if( m_names[i] != "" ) { m_names[i] = xpath( m_names[i] ).elements.back().to_string(); } }
}

} } }

#endif // COMMA_APPLICATION_NAME_VALUE_OPTIONS_H
