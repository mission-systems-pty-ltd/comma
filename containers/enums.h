// Copyright (c) 2024 Vsevolod Vlaskine

/// @author vsevolod vlaskine

#pragma once

#include <map>
#include <string>
#include <vector>
#include "find.h"

namespace comma { namespace enums {

/// trivial convenience wrapper; works only for enums with sequential values
template < typename Enum, typename K = std::string >
std::map< K, Enum > as_map( const std::vector< K >& keys, unsigned int begin = 0 );

/// trivial convenience wrapper; works only for enums with sequential values
template < typename Enum, typename K = std::string >
std::map< Enum, K > as_key_map( const std::vector< K >& keys, unsigned int begin = 0 );

/// trivial convenience wrapper
template < typename Enum, typename K = std::string >
Enum find( const K& k, const std::vector< K >& keys, unsigned int begin = 0 );

// todo: visiting traits
template < typename Enum, typename Names >
struct named: public Enum, Names
{
    const std::string& name() { return this->names()[static_cast< unsigned int >( *this )]; }
};


template < typename Enum, typename K >
inline std::map< K, Enum > as_map( const std::vector< K >& keys, unsigned int begin )
{
    std::map< K, Enum > m;
    for( unsigned int i{0}, j{begin}; i < keys.size(); ++i, ++j ) { m[keys[i]] = static_cast< Enum >( j ); }
    return m;
}

template < typename Enum, typename K >
inline std::map< Enum, K > as_key_map( const std::vector< K >& keys, unsigned int begin )
{
    std::map< Enum, K > m;
    for( unsigned int i{0}, j{begin}; i < keys.size(); ++i, ++j ) { m[static_cast< Enum >( j )] = keys[i]; }
    return m;
}

template < typename Enum, typename K >
Enum find( const K& k, const std::vector< K >& keys, unsigned int begin ) { return containers::find_or_throw< K, Enum >( as_map< Enum, K >( keys, begin ), k ); }

} } // namespace comma { namespace enums {
