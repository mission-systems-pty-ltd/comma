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

#ifndef COMMA_APPLICATION_NAME_VALUE_PARSER_H
#define COMMA_APPLICATION_NAME_VALUE_PARSER_H

#include <map>

#include <comma/base/exception.h>
#include <comma/visiting/apply.h>
#include <comma/name_value/map.h>
#include <comma/name_value/impl/options.h>
#include <comma/name_value/impl/from_name_value.h>
#include <comma/name_value/impl/to_name_value.h>

namespace comma
{
namespace name_value
{
    
/// parser for semicolon-separated name-value string
class parser
{
public:
    /// constructor
    /// @param delimiter delimiter between named values
    /// @param value_delimiter delimiter between name and value
    /// @param full_path_as_name use full path as name
    parser( char delimiter = ';', char value_delimiter = '=', bool full_path_as_name = true );
    
    /// constructor
    /// @param delimiter delimiter between named values
    /// @param value_delimiter delimiter between name and value
    /// @param full_path_as_name use full path as name
    /// @param fields default names for unnamed value
    parser( const std::string& fields, char delimiter = ';', char value_delimiter = '=', bool full_path_as_name = true );

    /// get struct from string
    template < typename S >
    S get( const std::string& line, const S& default_s = S() ) const;
    
    /// put struct into string
    template < typename S >
    std::string put( const S& s ) const;
    
    /// put struct into string
    /// @todo implement
    template < typename S >
    void put( std::string& line, const S& s ) const;

private:
    impl::options m_options;
};


inline parser::parser( char delimiter, char value_delimiter, bool full_path_as_name ):
    m_options( delimiter, value_delimiter, full_path_as_name )
{
}

inline parser::parser( const std::string& fields, char delimiter, char value_delimiter, bool full_path_as_name ):
    m_options( fields, delimiter, value_delimiter, full_path_as_name )
{
}

template < typename S >
inline S parser::get( const std::string& line, const S& default_s ) const
{
    map::map_type m = map( line, m_options ).get();
    name_value::impl::from_name_value from_name_value( m, m_options.m_full_path_as_name );
    S s = default_s;
    visiting::apply( from_name_value ).to( s );
    return s;
}

template < typename S >
inline std::string parser::put( const S& s ) const
{
    name_value::impl::to_name_value toname_value( m_options.m_value_delimiter, m_options.m_full_path_as_name );
    visiting::apply( toname_value ).to( s );
    return join( toname_value.strings(), m_options.m_delimiter );
}

} }

#endif // COMMA_APPLICATION_NAME_VALUE_PARSER_H
