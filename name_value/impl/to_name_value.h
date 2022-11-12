// Copyright (c) 2011 The University of Sydney
// Copyright (c) 2022 Vsevolod Vlaskine

/// @authors cedric wohlleber, vsevolod vlaskine

#pragma once

#include <map>
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
    to_name_value( char delimiter = '=', bool full_path_as_name = true ): m_delimiter(delimiter), m_full_path_as_name( full_path_as_name ) {};

    /// apply
    template < typename K, typename T > void apply( const K& name, const boost::optional< T >& value ) { if( value ) { apply( name, *value ); } }
    
    /// apply
    template < typename K, typename T > void apply( const K& name, const boost::scoped_ptr< T >& value ) { if( value ) { apply( name, *value ); } }
    
    /// apply
    template < typename K, typename T > void apply( const K& name, const boost::shared_ptr< T >& value ) { if( value ) { apply( name, *value ); } }
        
    /// apply
    template < typename K, typename T > void apply( const K& name, const T& value );

    /// apply to non-leaf elements
    template < typename K, typename T > void apply_next( const K& name, const T& value ) { comma::visiting::visit( name, value, *this ); }

    /// apply to leaf elements
    template < typename K, typename T > void apply_final( const K& name, const T& value );

    /// return named values as strings
    const std::vector< std::string >& strings() const { return m_strings; }

private:
    template < typename T > std::string as_string( T v )
    {
        std::ostringstream oss;
        oss << v;
        return oss.str();
    }
        
    char m_delimiter;
    bool m_full_path_as_name;
    std::vector< std::string > m_strings;
    xpath m_xpath;
     
};

template < typename K, typename T >
inline void to_name_value::apply( const K& name, const T& value )
{
    m_xpath /= xpath::element( name );
    visiting::do_while<    !boost::is_fundamental< T >::value
                        && !boost::is_same< T, std::string >::value >::visit( name, value, *this );
    m_xpath = m_xpath.head();
}

template < typename K, typename T >
inline void to_name_value::apply_final( const K&, const T& value ) { m_strings.push_back( std::string( m_full_path_as_name ? m_xpath.to_string() : m_xpath.elements.back().to_string() ) + m_delimiter + as_string( value ) ); }

} } } // namespace comma { namespace name_value { namespace impl {
