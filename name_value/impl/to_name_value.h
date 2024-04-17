// Copyright (c) 2011 The University of Sydney
// Copyright (c) 2022 Vsevolod Vlaskine

/// @authors cedric wohlleber, vsevolod vlaskine

#pragma once

#include <map>
#include <memory>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/type_traits.hpp>
#include "../../string/string.h"
#include "../../visiting/while.h"
#include "../../visiting/visit.h"
#include "../../xpath/xpath.h"

namespace comma { namespace name_value { namespace impl {

/// visitor getting a string from a struct 
class to_name_value
{
public:
    /// constructor
    /// @param delimiter delimiter between name and value
    /// @param full_path_as_name use full path as name
    to_name_value( char delimiter = '=', bool full_path_as_name = true ): _delimiter(delimiter), _full_path_as_name( full_path_as_name ) {};
    template < typename K, typename T > void apply( const K& name, const boost::optional< T >& value ) { if( value ) { apply( name, *value ); } }
    template < typename K, typename T > void apply( const K& name, const std::optional< T >& value ) { if( value ) { apply( name, *value ); } }
    template < typename K, typename T > void apply( const K& name, const boost::scoped_ptr< T >& value ) { if( value ) { apply( name, *value ); } }
    template < typename K, typename T > void apply( const K& name, const boost::shared_ptr< T >& value ) { if( value ) { apply( name, *value ); } }
    template < typename K, typename T > void apply( const K& name, const std::unique_ptr< T >& value ) { if( value ) { apply( name, *value ); } }
    template < typename K, typename T > void apply( const K& name, const T& value );
    template < typename K, typename T > void apply_next( const K& name, const T& value ) { comma::visiting::visit( name, value, *this ); }
    template < typename K, typename T > void apply_final( const K& name, const T& value );
    const std::vector< std::string >& strings() const { return _strings; }

private:
    char _delimiter;
    bool _full_path_as_name;
    std::vector< std::string > _strings;
    xpath _xpath;

    template < typename T > std::string _as_string( T v ) { std::ostringstream oss; oss << v; return oss.str(); } 
};

template < typename K, typename T >
inline void to_name_value::apply( const K& name, const T& value )
{
    _xpath /= xpath::element( name );
    visiting::do_while<    !boost::is_fundamental< T >::value
                        && !boost::is_same< T, std::string >::value >::visit( name, value, *this );
    _xpath = _xpath.head();
}

template < typename K, typename T >
inline void to_name_value::apply_final( const K&, const T& value ) { _strings.push_back( std::string( _full_path_as_name ? _xpath.to_string() : _xpath.elements.back().to_string() ) + _delimiter + _as_string( value ) ); }

} } } // namespace comma { namespace name_value { namespace impl {
