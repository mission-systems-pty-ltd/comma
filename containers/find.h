// Copyright (c) 2024 Vsevolod Vlaskine

/// @author vsevolod vlaskine

#pragma once

#include <map>
#include <unordered_map>
#include "../base/exception.h"

namespace comma { namespace containers {

/// trivial convenience wrapper: find element, if not found, throw exception
template < typename K, typename V > const V& find_or_throw( const std::map< K, V >& m, const K& k, const std::string& message = std::string() );

/// trivial convenience wrapper: find element, if not found, throw exception
template < typename K, typename V > V& find_or_throw( std::map< K, V >& m, const K& k, const std::string& message = std::string() );

/// trivial convenience wrapper: find element, if not found, throw exception
template < typename K, typename V > const V& find_or_throw( const std::unordered_map< K, V >& m, const K& k, const std::string& message = std::string() );

/// trivial convenience wrapper: find element, if not found, throw exception
template < typename K, typename V > V& find_or_throw( std::unordered_map< K, V >& m, const K& k, const std::string& message = std::string() );


namespace impl {

template < typename M, typename K > inline const auto& find_or_throw( const M& m, const K& k, const std::string& message )
{
    auto it = m.find( k );
    COMMA_ASSERT( it != m.end(), ( message.empty() ? std::string() : ( message + ": " ) ) << "k '" << k << "' not found" );
    return it->second;
}

template < typename M, typename K > inline auto& find_or_throw( M& m, const K& k, const std::string& message )
{
    auto it = m.find( k );
    COMMA_ASSERT( it != m.end(), ( message.empty() ? std::string() : ( message + ": " ) ) << "k '" << k << "' not found" );
    return it->second;
}

} // namespace impl {

template < typename K, typename V > inline const V& find_or_throw( const std::map< K, V >& m, const K& k, const std::string& message ) { return impl::find_or_throw( m, k, message ); }

template < typename K, typename V > inline V& find_or_throw( std::map< K, V >& m, const K& k, const std::string& message ) { return impl::find_or_throw( m, k, message ); }

template < typename K, typename V > inline const V& find_or_throw( const std::unordered_map< K, V >& m, const K& k, const std::string& message ) { return impl::find_or_throw( m, k, message ); }

template < typename K, typename V > inline V& find_or_throw( std::unordered_map< K, V >& m, const K& k, const std::string& message ) { return impl::find_or_throw( m, k, message ); }

} } // namespace comma { namespace containers {
