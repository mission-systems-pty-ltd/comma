// Copyright (c) 2011 The University of Sydney

/// @author vsevolod vlaskine

#pragma once

#ifndef WIN32
#include <stdlib.h>
#endif
#include <array>
#include <map>
#include <set>
#include <string>
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
