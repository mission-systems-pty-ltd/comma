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


/// @authors cedric wohlleber, vsevolod vlaskine

#pragma once

#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_parsers.hpp>
#include "impl/options.h"

namespace comma { namespace name_value {

/// constructs a map of name-value pair from an input string
/// TODO implement full_path_as_name ? 
class map
{
    public:
        /// constructor
        map( const std::string& line, char delimiter = ';', char value_delimiter = '=' );
        /// constructor
        map( const std::string& line, const std::string& fields, char delimiter = ';', char value_delimiter = '=' );
        /// constructor
        map( const std::string& line, const impl::options& options );
        
        /// return vector of name-value pairs in the given order
        static std::vector< std::pair< std::string, std::string > > as_vector( const std::string& line, char delimiter = ';', char value_delimiter = '=' );
        
        /// return vector of name-value pairs in the given order
        static std::vector< std::pair< std::string, std::string > > as_vector( const std::string& line, const std::string& fields, char delimiter = ';', char value_delimiter = '=' );
        
        /// return vector of name-value pairs in the given order
        static std::vector< std::pair< std::string, std::string > > as_vector( const std::string& line, const impl::options& options );

        /// return true, if field exists
        bool exists( const std::string& name ) const;

        /// return values for all the fields with a given name
        template < typename T >
        std::vector< T > values( const std::string& name ) const;

        /// return value, if field exists; otherwise return default
        template < typename T >
        T value( const std::string& name, const T& default_value ) const;

        /// return value, if field exists; otherwise throw
        template < typename T >
        T value( const std::string& name ) const;

        /// map type
        typedef std::multimap< std::string, std::string > map_type;

        /// return name-value map
        const map_type& get() const { return m_map; }

    private:
        void init_( const comma::name_value::impl::options& options );
        const std::string m_line;
        map_type m_map;
};

inline map::map( const std::string& line, char delimiter, char value_delimiter ): m_line( line ) { init_( impl::options( delimiter, value_delimiter ) ); }

inline map::map( const std::string& line, const std::string& fields, char delimiter, char value_delimiter ): m_line( line ) { init_( impl::options( fields, delimiter, value_delimiter ) ); }

inline map::map( const std::string& line, const comma::name_value::impl::options& options ): m_line( line ) { init_( options ); }

inline static std::vector< std::string > get_named_values( const std::string& line, const comma::name_value::impl::options& options )
{
    std::vector< std::string > named_values = split_escaped( line, options.m_delimiter, &(options.m_quotes[0]), options.m_escape );
    for( std::size_t i = 0; i < options.m_names.size() && i < named_values.size(); ++i )
    {
        if( options.m_names[i].empty() ) { continue; }
        if( split( named_values[i], options.m_value_delimiter ).size() != 1U ) { COMMA_THROW_STREAM( comma::exception, "expected unnamed value for " << options.m_names[i] << ", got: " << named_values[i] ); }
        named_values[i] = options.m_names[i] + options.m_value_delimiter + named_values[i];
    }
    return named_values;
}

inline void map::init_( const comma::name_value::impl::options& options )
{
    const std::vector< std::string >& named_values = get_named_values( m_line, options );
    for( std::size_t i = 0; i < named_values.size(); ++i )
    {
        std::vector< std::string > pair = split_escaped( named_values[i], options.m_value_delimiter, &(options.m_quotes[0]), options.m_escape );
        switch( pair.size() )
        {
            case 1: m_map.insert( std::make_pair( pair[0], std::string() ) ); break; // quick and dirty
            case 2: m_map.insert( std::make_pair( pair[0], pair[1] ) ); break;
            default: { COMMA_THROW_STREAM( comma::exception, "expected name-value pair, got: " << join( pair, options.m_value_delimiter ) ); }
        }
    }
}

inline std::vector< std::pair< std::string, std::string > > map::as_vector( const std::string& line, char delimiter, char value_delimiter ) { return as_vector( line, impl::options( delimiter, value_delimiter ) ); } 

inline std::vector< std::pair< std::string, std::string > > map::as_vector( const std::string& line, const std::string& fields, char delimiter, char value_delimiter ) { return as_vector( line, impl::options( fields, delimiter, value_delimiter ) ); }

inline std::vector< std::pair< std::string, std::string > > map::as_vector( const std::string& line, const impl::options& options )
{
    std::vector< std::pair< std::string, std::string > > v;
    const std::vector< std::string >& named_values = get_named_values( line, options );
    for( std::size_t i = 0; i < named_values.size(); ++i )
    {
        std::vector< std::string > pair = split_escaped( named_values[i], options.m_value_delimiter, &(options.m_quotes[0]), options.m_escape );
        switch( pair.size() )
        {
            case 1: v.push_back( std::make_pair( pair[0], std::string() ) ); break; // quick and dirty
            case 2: v.push_back( std::make_pair( pair[0], pair[1] ) ); break;
            default: { COMMA_THROW_STREAM( comma::exception, "expected name-value pair, got: " << join( pair, options.m_value_delimiter ) ); }
        }
    }
    return v;
}

inline bool map::exists( const std::string& name ) const { return m_map.find( name ) != m_map.end(); }

namespace detail {

template < typename T >
inline T lexical_cast( const std::string& s ) { return boost::lexical_cast< T >( s ); }

template <>
inline bool lexical_cast< bool >( const std::string& s )
{
    if( s == "" || s == "true" ) { return true; }
    if( s == "false" ) { return false; }
    return boost::lexical_cast< bool >( s );
}

template <>
inline boost::posix_time::ptime lexical_cast< boost::posix_time::ptime >( const std::string& s )
{
    if ( s == "not-a-date-time" ) { return boost::posix_time::not_a_date_time; }
    else if ( s == "+infinity" || s == "+inf" || s == "inf" ) { return boost::posix_time::pos_infin; }
    else if ( s == "-infinity" || s == "-inf" ) { return boost::posix_time::neg_infin; }
    else return boost::posix_time::from_iso_string( s );
}

} // namespace detail {

template < typename T >
inline std::vector< T > map::values( const std::string& name ) const
{
    std::vector< T > v;
    for( typename map_type::const_iterator it = m_map.begin(); it != m_map.end(); ++it )
    {
        if( it->first == name ) { v.push_back( detail::lexical_cast< T >( it->second ) ); }
    }
    return v;
}

template < typename T >
inline T map::value( const std::string& name, const T& default_value ) const
{
    const std::vector< T >& v = values< T >( name );
    return v.empty() ? default_value : v[0];
}

template < typename T >
inline T map::value( const std::string& name ) const
{
    const std::vector< T >& v = values< T >( name );
    if( v.empty() ) { COMMA_THROW_STREAM( comma::exception, "'" << name << "' not found in \"" << m_line << "\"" ); }
    return v[0];
}

} } // namespace comma { namespace name_value {
