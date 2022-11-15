// Copyright (c) 2011 The University of Sydney
// Copyright (c) 2022 Vsevolod Vlaskine

/// @author vsevolod vlaskine

#pragma once

#ifndef WIN32
#include <stdlib.h>
#endif
#include <array>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <boost/array.hpp>
#include "../base/none.h"

namespace comma { namespace visiting {

/// visiting traits, need to be specialized for visited classes;
/// see visiting and serialization unit tests for examples
template < typename T >
struct traits
{
    template < typename K, typename V > static void visit( const K& key, T& t, V& v );
    template < typename K, typename V > static void visit( const K& key, const T& t, V& v );
};

template <> struct traits< comma::none >
{
    template < typename K, typename V > static void visit( const K& key, comma::none& t, V& v ) {}
    template < typename K, typename V > static void visit( const K&, const comma::none& t, V& v ) {}
};

template < typename T, typename S >
struct traits< std::pair< T, S > >
{
    template < typename K, typename V > static void visit( const K& key, std::pair< T, S >& t, V& v )
    {
        v.apply( "first", t.first );
        v.apply( "second", t.second );
    }
    
    template < typename K, typename V > static void visit( const K&, const std::pair< T, S >& t, V& v )
    {
        v.apply( "first", t.first );
        v.apply( "second", t.second );
    }    
};

namespace detail {

// template < unsigned int I > const char* element_name(); // super-quick and dirty for now, it would be better to use numeric indices, but then all visitors need to support tuple correctly
// template <> inline const char* element_name< 0 >() { return "first"; }
// template <> inline const char* element_name< 1 >() { return "second"; }
// template <> inline const char* element_name< 2 >() { return "third"; }
// template <> inline const char* element_name< 3 >() { return "fourth"; }
// template <> inline const char* element_name< 4 >() { return "fifth"; }
// template <> inline const char* element_name< 5 >() { return "sixth"; }
// template <> inline const char* element_name< 6 >() { return "seventh"; }
// template <> inline const char* element_name< 7 >() { return "eighth"; }

// template < unsigned int I, unsigned int Size > struct elementwise
// {
//     template < typename T, typename V > static void visit( T& t, V& v ) { v.apply( element_name< Size - I >(), std::get< Size - I >( t ) ); elementwise< I - 1, Size >::visit( t, v ); }
//     template < typename T, typename V > static void visit( const T& t, V& v ) { v.apply( element_name< Size - I >(), std::get< Size - I >( t ) ); elementwise< I - 1, Size >::visit( t, v ); }
// };

// template < unsigned int Size > struct elementwise< 1, Size >
// {
//     template < typename T, typename V > static void visit( T& t, V& v ) { v.apply( element_name< Size - 1 >(), std::get< Size - 1 >( t ) ); }
//     template < typename T, typename V > static void visit( const T& t, V& v ) { v.apply( element_name< Size - 1 >(), std::get< Size - 1 >( t ) ); }
// };

template < unsigned int I, unsigned int Size > struct elementwise // todo! add tuple support to all visitors
{
    template < typename T, typename V > static void visit( T& t, V& v ) { v.apply( Size - I, std::get< Size - I >( t ) ); elementwise< I - 1, Size >::visit( t, v ); }
    template < typename T, typename V > static void visit( const T& t, V& v ) { v.apply( Size - I, std::get< Size - I >( t ) ); elementwise< I - 1, Size >::visit( t, v ); }
};

template < unsigned int Size > struct elementwise< 1, Size >
{
    template < typename T, typename V > static void visit( T& t, V& v ) { v.apply( Size - 1, std::get< Size - 1 >( t ) ); }
    template < typename T, typename V > static void visit( const T& t, V& v ) { v.apply( Size - 1, std::get< Size - 1 >( t ) ); }
};

} // namespace detail {

template < typename... T >
struct traits< std::tuple< T... > >
{
    typedef std::tuple< T... > tuple_t;

    static const unsigned int size{ std::tuple_size< tuple_t >::value };

    static_assert( size > 0 );

    template < typename K, typename V > static void visit( const K& key, tuple_t& t, V& v ) { detail::elementwise< size, size >::visit( t, v ); }
    
    template < typename K, typename V > static void visit( const K&, const tuple_t& t, V& v ) { detail::elementwise< size, size >::visit( t, v ); }
};

namespace impl {

template < typename K, typename V, typename Visitor >
inline void visit_non_associative_container( const K&, const V& c, Visitor& v )
{
    std::size_t index = 0;
    for( typename V::const_iterator it = c.begin(); it != c.end(); ++it, ++index ) { v.apply( index, *it ); }
}

template < typename K, typename V, typename Visitor >
inline void visit_non_associative_container( const K&, V& c, Visitor& v )
{
    std::size_t index = 0;
    for( typename V::iterator it = c.begin(); it != c.end(); ++it, ++index ) { v.apply( index, *it ); }
}

template < typename T, typename Visitor >
inline void visit_associative_container_key( const std::string& k, T& t, Visitor& v ) { v.apply( &k[0], t ); }

template < typename T, typename Visitor >
inline void visit_associative_container_key( const std::string& k, const T& t, Visitor& v ) { v.apply( &k[0], t ); }

template < typename K, typename T, typename Visitor >
inline void visit_associative_container_key( const K& k, T& t, Visitor& v ) { v.apply( k, t ); }

template < typename K, typename T, typename Visitor >
inline void visit_associative_container_key( const K& k, const T& t, Visitor& v ) { v.apply( k, t ); }

template < typename K, typename M, typename Visitor >
inline void visit_associative_container( const K&, M& c, Visitor& v )
{
    for( typename M::iterator it = c.begin(); it != c.end(); ++it ) { visit_associative_container_key( it->first, it->second, v ); }
}

template < typename K, typename M, typename Visitor >
inline void visit_associative_container( const K&, const M& c, Visitor& v )
{
    for( typename M::const_iterator it = c.begin(); it != c.end(); ++it ) { visit_associative_container_key( it->first, it->second, v ); }
}

} // namespace impl {

template < typename T, typename A > struct traits< std::vector< T, A > >
{
    template < typename K, typename V > static void visit( const K& key, std::vector< T, A >& t, V& v ) { impl::visit_non_associative_container( key, t, v ); }
    template < typename K, typename V > static void visit( const K& key, const std::vector< T, A >& t, V& v ) { impl::visit_non_associative_container( key, t, v ); }    
};

template < typename T > struct traits< std::set< T > >
{
    template < typename K, typename V > static void visit( const K& key, std::set< T >& t, V& v ) { impl::visit_non_associative_container( key, t, v ); }
    template < typename K, typename V > static void visit( const K& key, const std::set< T >& t, V& v ) { impl::visit_non_associative_container( key, t, v ); }    
};

template < typename T > struct traits< std::unordered_set< T > >
{
    template < typename K, typename V > static void visit( const K& key, std::unordered_set< T >& t, V& v ) { impl::visit_non_associative_container( key, t, v ); }
    template < typename K, typename V > static void visit( const K& key, const std::unordered_set< T >& t, V& v ) { impl::visit_non_associative_container( key, t, v ); }    
};

template < typename T, std::size_t S > struct traits< std::array< T, S > >
{
    template < typename K, typename V > static void visit( const K& key, std::array< T, S >& t, V& v ) { impl::visit_non_associative_container( key, t, v ); }
    template < typename K, typename V > static void visit( const K& key, const std::array< T, S >& t, V& v ) { impl::visit_non_associative_container( key, t, v ); }
};

template < typename T, std::size_t S > struct traits< boost::array< T, S > >
{
    template < typename K, typename V > static void visit( const K& key, boost::array< T, S >& t, V& v ) { impl::visit_non_associative_container( key, t, v ); }
    template < typename K, typename V > static void visit( const K& key, const boost::array< T, S >& t, V& v ) { impl::visit_non_associative_container( key, t, v ); }    
};

template < typename T, typename S > struct traits< std::map< T, S > >
{
    template < typename K, typename V > static void visit( const K& key, std::map< T, S >& t, V& v ) { impl::visit_associative_container( key, t, v ); }
    template < typename K, typename V > static void visit( const K& key, const std::map< T, S >& t, V& v ) { impl::visit_associative_container( key, t, v ); }    
};

template < typename T, typename S > struct traits< std::unordered_map< T, S > >
{
    template < typename K, typename V > static void visit( const K& key, std::unordered_map< T, S >& t, V& v ) { impl::visit_associative_container( key, t, v ); }
    template < typename K, typename V > static void visit( const K& key, const std::unordered_map< T, S >& t, V& v ) { impl::visit_associative_container( key, t, v ); }    
};

/// @todo add more types as needed

} } // namespace comma { namespace visiting {
