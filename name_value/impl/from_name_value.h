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
// 3. All advertising materials mentioning features or use of this software
//    must display the following acknowledgement:
//    This product includes software developed by the University of Sydney.
// 4. Neither the name of the University of Sydney nor the
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

#ifndef COMMA_APPLICATION_FROM_NAME_VALUE_H
#define COMMA_APPLICATION_FROM_NAME_VALUE_H

#include <deque>
#include <map>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/type_traits.hpp>
#include <comma/base/types.h>
#include <comma/string/string.h>
#include <comma/visiting/while.h>
#include <comma/visiting/visit.h>
#include <comma/xpath/xpath.h>

namespace comma { namespace name_value { namespace impl {

/// visitor getting a struct from named values
class from_name_value
{
public:
    /// map type
    typedef std::multimap< std::string, std::string > map_type;
    
    /// constructor
    /// @param values values to read from
    /// @param full_path_as_name use full path as name
    from_name_value( const map_type& values, bool full_path_as_name = true ):
        m_values( values ), m_full_path_as_name(full_path_as_name){};

    /// apply
    template < typename K, typename T > void apply( const K& name, boost::optional< T >& value );
    
    /// apply
    template < typename K, typename T > void apply( const K& name, boost::scoped_ptr< T >& value );
    
    /// apply
    template < typename K, typename T > void apply( const K& name, boost::shared_ptr< T >& value );
        
    /// apply
    template < typename K, typename T > void apply( const K& name, T& value );

    /// apply to non-leaf elements
    template < typename K, typename T > void apply_next( const K& name, T& value );

    /// apply to leaf elements
    template < typename K, typename T > void apply_final( const K& name, T& value );

private:
    const map_type& m_values;
    bool m_full_path_as_name;
    xpath m_xpath;
    std::deque< bool > m_empty;
    static void lexical_cast( bool& v, const std::string& s ) { v = s == "" || boost::lexical_cast< bool >( s ); }
    static void lexical_cast( boost::posix_time::ptime& v, const std::string& s ) { v = boost::posix_time::from_iso_string( s ); }
    static void lexical_cast( boost::posix_time::time_duration& v, const std::string& s )
    {
        std::vector< std::string > t = comma::split( s, '.' );
        if( t.size() > 2 ) { COMMA_THROW_STREAM( comma::exception, "expected duration in seconds, got " << s ); }
        comma::int64 seconds = boost::lexical_cast< comma::int64 >( t[0] );
        if( t[1].length() > 6 ) { t[1] = t[1].substr( 0, 6 ); }
        else { t[1] += std::string( 6 - t[1].length(), '0' ); }
        comma::int32 microseconds = boost::lexical_cast< comma::int32 >( t[1] );
        if( seconds < 0 ) { microseconds = -microseconds; }
        v = boost::posix_time::seconds( seconds ) + boost::posix_time::microseconds( microseconds );
    }
    template < typename T > static void lexical_cast( T& v, const std::string& s ) { v = boost::lexical_cast< T >( s ); }
};

template < typename K, typename T >
inline void from_name_value::apply( const K& name, boost::optional< T >& value )
{
    if( value ) { apply( name, *value ); return; }
    T t;
    m_empty.push_back( true );
    apply( name, t );
    if( !m_empty.back() ) { value = t; }
    m_empty.pop_back();
}

template < typename K, typename T >
inline void from_name_value::apply( const K& name, boost::scoped_ptr< T >& value )
{
    if( value ) { apply( name, *value ); return; }
    T t;
    m_empty.push_back( true );
    apply( name, t );
    if( !m_empty.back() ) { value.reset( new T( t ) ); }
    m_empty.pop_back();
}

template < typename K, typename T >
inline void from_name_value::apply( const K& name, boost::shared_ptr< T >& value )
{
    if( value ) { apply( name, *value ); return; }
    T t;
    m_empty.push_back( true );
    apply( name, t );
    if( !m_empty.back() ) { value.reset( new T( t ) ); }
    m_empty.pop_back();
}

template < typename K, typename T >
inline void from_name_value::apply( const K& name, T& value )
{
    m_xpath /= xpath::element( name );
    visiting::do_while<    !boost::is_fundamental< T >::value
                        && !boost::is_same< T, boost::posix_time::ptime >::value
                        && !boost::is_same< T, boost::posix_time::time_duration >::value
                        && !boost::is_same< T, std::string >::value >::visit( name, value, *this );
    m_xpath = m_xpath.head();
}

template < typename K, typename T >
inline void from_name_value::apply_next( const K& name, T& value ) { comma::visiting::visit( name, value, *this ); }

template < typename K, typename T >
inline void from_name_value::apply_final( const K& key, T& value )
{
    map_type::const_iterator iter = m_values.find( m_full_path_as_name ? m_xpath.to_string() : m_xpath.elements.back().to_string() );
    if( iter == m_values.end() ) { return; }
    lexical_cast( value, iter->second );
    for( std::size_t i = 0; i < m_empty.size(); ++i ) { m_empty[i] = false; }
}

} } } // namespace comma { namespace name_value { namespace impl {

#endif // COMMA_APPLICATION_FROM_NAME_VALUE_H
