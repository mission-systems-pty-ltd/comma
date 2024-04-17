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

#pragma once

#include <deque>
#include <map>
#include <memory>
#if __cplusplus >= 201703L
#include <optional>
#endif // #if __cplusplus >= 201703L
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/type_traits.hpp>
#include "../../base/types.h"
#include "../../string/string.h"
#include "../../visiting/while.h"
#include "../../visiting/visit.h"
#include "../../xpath/xpath.h"

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
    from_name_value( const map_type& values, bool full_path_as_name = true ): _values( values ), _full_path_as_name( full_path_as_name ) {};

    template < typename K, typename T > void apply( const K& name, boost::optional< T >& value ) { _apply_optional< K, T >( name, value ); }

    #if __cplusplus >= 201703L
    template < typename K, typename T > void apply( const K& name, std::optional< T >& value ) { _apply_optional< K, T >( name, value ); }
    #endif // #if __cplusplus >= 201703L
    
    template < typename K, typename T > void apply( const K& name, boost::scoped_ptr< T >& value ) { _apply_ptr< K, T >( name, value ); }
    
    template < typename K, typename T > void apply( const K& name, boost::shared_ptr< T >& value ) { _apply_ptr< K, T >( name, value ); }

    template < typename K, typename T > void apply( const K& name, std::unique_ptr< T >& value ) { _apply_ptr< K, T >( name, value ); }
        
    template < typename K, typename T > void apply( const K& name, T& value );

    template < typename K, typename T > void apply_next( const K& name, T& value );

    template < typename K, typename T > void apply_final( const K& name, T& value );

private:
    const map_type& _values;
    bool _full_path_as_name;
    xpath _xpath;
    std::deque< bool > _empty;
    static void _lexical_cast( bool& v, const std::string& s ) { v = s == "" || boost::lexical_cast< bool >( s ); }
    static void _lexical_cast( boost::posix_time::ptime& v, const std::string& s ) { v = boost::posix_time::from_iso_string( s ); }
    static void _lexical_cast( boost::posix_time::time_duration& v, const std::string& s )
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
    template < typename T > static void _lexical_cast( T& v, const std::string& s ) { v = boost::lexical_cast< T >( s ); }
    template < typename K, typename T, template < typename > class Optional > void _apply_optional( const K& name, Optional< T >& value );
    template < typename K, typename T, template < typename > class Ptr > void _apply_ptr( const K& name, Ptr< T >& value );
};

template < typename K, typename T, template < typename > class Optional > inline void from_name_value::_apply_optional( const K& name, Optional< T >& value )
{
    if( value ) { apply( name, *value ); return; }
    T t;
    _empty.push_back( true );
    apply( name, t );
    if( !_empty.back() ) { value = t; }
    _empty.pop_back();    
}

template < typename K, typename T, template < typename > class Ptr > inline void from_name_value::_apply_ptr( const K& name, Ptr< T >& value )
{
    if( value ) { apply( name, *value ); return; }
    T t;
    _empty.push_back( true );
    apply( name, t );
    if( !_empty.back() ) { value.reset(); value.reset( new T( t ) ); } // todo? emplace? 
    _empty.pop_back();
}

template < typename K, typename T > inline void from_name_value::apply( const K& name, T& value )
{
    _xpath /= xpath::element( name );
    visiting::do_while<    !boost::is_fundamental< T >::value
                        && !boost::is_same< T, boost::posix_time::ptime >::value
                        && !boost::is_same< T, boost::posix_time::time_duration >::value
                        && !boost::is_same< T, std::string >::value >::visit( name, value, *this );
    _xpath = _xpath.head();
}

template < typename K, typename T > inline void from_name_value::apply_next( const K& name, T& value ) { comma::visiting::visit( name, value, *this ); }

template < typename K, typename T > inline void from_name_value::apply_final( const K& key, T& value )
{
    map_type::const_iterator iter = _values.find( _full_path_as_name ? _xpath.to_string() : _xpath.elements.back().to_string() );
    if( iter == _values.end() ) { return; }
    _lexical_cast( value, iter->second );
    for( std::size_t i = 0; i < _empty.size(); ++i ) { _empty[i] = false; }
}

} } } // namespace comma { namespace name_value { namespace impl {
