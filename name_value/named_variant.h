// This file is part of comma, a generic and flexible library
// Copyright (c) 2026 Vsevolod Vlaskine
// All rights reserved.

/// @author vsevolod vlaskine

#pragma once

#include <string>
#include <unordered_map>
#include "../base/optional.h"
#include "../base/variant.h"
#include "../name_value/parser.h"
#include "../name_value/ptree.h"
#include "../string/string.h"

namespace comma { namespace name_value { namespace named_variant {

template < typename NamedVariant >
typename comma::optional< NamedVariant >::type get( const std::string& s, char equal_sign = '=', char delimiter = ',' );


namespace impl {

using map_t = std::unordered_map< std::string, std::string >;

template< typename T, typename = std::void_t<> > struct naming_traits { static map_t alias_map() { return map_t{}; } };

template< typename T > struct naming_traits< T, std::void_t< decltype( T::alias_map() ) > > { static map_t alias_map() { return T::alias_map(); } };

} // namespace impl {

template < typename NamedVariant >
inline typename comma::optional< NamedVariant >::type get( const std::string& t, char equal_sign, char delimiter )
{
    std::string s = comma::strip( t );
    boost::property_tree::ptree p;
    if( s[0] == '{' ) // todo: hyper-quick and dirty for now; implement at someplace generic
    {
        std::stringstream iss( s );
        boost::property_tree::read_json( iss, p );
    }
    else
    {
        p = comma::property_tree::from_path_value_string( comma::name_value::parser::mangled( comma::name_value::parser::unaliased( s, impl::naming_traits< typename NamedVariant::names_t >::alias_map(), delimiter ) + delimiter + "_dummy" + equal_sign + "0", delimiter ), equal_sign, delimiter ); // quick and dirty: add dummy to get a non-empty json section for parameter-less commands
    }
    comma::from_ptree from_ptree( p, true );
    NamedVariant v;
    comma::visiting::apply( from_ptree ).to( v );
    return v;
}

} } } // namespace comma { namespace name_value { namespace named_variant {
