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

#ifndef COMMA_APPLICATION_TO_NAME_VALUE_H
#define COMMA_APPLICATION_TO_NAME_VALUE_H

#include <map>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/type_traits.hpp>
#include <comma/string/string.h>
#include <comma/visiting/while.h>
#include <comma/visiting/visit.h>
#include <comma/xpath/xpath.h>

namespace comma { namespace name_value { namespace impl {

/// visitor getting a string from a struct 
class to_name_value
{
public:
    /// constructor
    /// @param delimiter delimiter between name and value
    /// @param full_path_as_name use full path as name
    to_name_value( char delimiter = '=', bool full_path_as_name = true ):
        m_delimiter(delimiter), m_full_path_as_name(full_path_as_name){};

    /// apply
    template < typename K, typename T > void apply( const K& name, const boost::optional< T >& value );
    
    /// apply
    template < typename K, typename T > void apply( const K& name, const boost::scoped_ptr< T >& value );
    
    /// apply
    template < typename K, typename T > void apply( const K& name, const boost::shared_ptr< T >& value );        
        
    /// apply
    template < typename K, typename T >
    void apply( const K& name, const T& value );

    /// apply to non-leaf elements
    template < typename K, typename T >
    void apply_next( const K& name, const T& value );

    /// apply to leaf elements
    template < typename K, typename T >
    void apply_final( const K& name, const T& value );

    /// return named values as strings
    const std::vector< std::string >& strings() const { return m_strings; }

private:
    template < typename T >
        std::string as_string( T v )
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
inline void to_name_value::apply( const K& name, const boost::optional< T >& value )
{
    if( value ) { apply( name, *value ); }
}

template < typename K, typename T >
inline void to_name_value::apply( const K& name, const boost::scoped_ptr< T >& value )
{
    if( value ) { apply( name, *value ); }
}

template < typename K, typename T >
inline void to_name_value::apply( const K& name, const boost::shared_ptr< T >& value )
{
    if( value ) { apply( name, *value ); }
}

template < typename K, typename T >
inline void to_name_value::apply( const K& name, const T& value )
{
    m_xpath /= xpath::element( name );
    visiting::do_while<    !boost::is_fundamental< T >::value
                        && !boost::is_same< T, std::string >::value >::visit( name, value, *this );
    m_xpath = m_xpath.head();
}



template < typename K, typename T >
inline void to_name_value::apply_next( const K& name, const T& value ) { comma::visiting::visit( name, value, *this ); }

template < typename K, typename T >
inline void to_name_value::apply_final( const K&, const T& value )
{
    std::string string;
    if( m_full_path_as_name )
    {
        string += m_xpath.to_string();
    }
    else
    {
        string += m_xpath.elements.back().to_string();
    }
    string += m_delimiter + as_string( value );
    m_strings.push_back( string );
}

} } } // namespace comma { namespace name_value { namespace impl {

#endif // COMMA_APPLICATION_TO_NAME_VALUE_H
