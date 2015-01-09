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


/// @author vsevolod vlaskine

#ifndef COMMA_VISITING_TRAITS_HEADER_GUARD_
#define COMMA_VISITING_TRAITS_HEADER_GUARD_

#ifndef WIN32
#include <stdlib.h>
#endif
#include <map>
#include <set>
#include <string>
#include <vector>
#include <boost/array.hpp>

namespace comma { namespace visiting {

/// visiting traits, need to be specialized for visited classes;
/// see visiting and serialization unit tests for examples
template < typename T >
struct traits
{
    /// visit arbitrary type
    template < typename K, typename V > static void visit( const K& key, T& t, V& v );
};

/// std::pair visiting traits; todo: better semantics?
template < typename T, typename S >
struct traits< std::pair< T, S > >
{
    /// visit
    template < typename K, typename V > static void visit( const K& key, std::pair< T, S >& t, V& v )
    {
        v.apply( "first", t.first );
        v.apply( "second", t.second );
    }
    
    /// visit const
    template < typename K, typename V > static void visit( const K&, const std::pair< T, S >& t, V& v )
    {
        v.apply( "first", t.first );
        v.apply( "second", t.second );
    }    
};

namespace Impl {

template < typename K, typename V, typename Visitor >
inline void visit_non_associative_container( const K&, const V& c, Visitor& v )
{
    std::size_t index = 0;
    for( typename V::const_iterator it = c.begin(); it != c.end(); ++it, ++index )
    {
        v.apply( index, *it );
    }
}

template < typename K, typename V, typename Visitor >
inline void visit_non_associative_container( const K&, V& c, Visitor& v )
{
    std::size_t index = 0;
    for( typename V::iterator it = c.begin(); it != c.end(); ++it, ++index )
    {
        v.apply( index, *it );
    }
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
    for( typename M::iterator it = c.begin(); it != c.end(); ++it )
    {
        visit_associative_container_key( it->first, it->second, v );
    }
}

template < typename K, typename M, typename Visitor >
inline void visit_associative_container( const K&, const M& c, Visitor& v )
{
    for( typename M::const_iterator it = c.begin(); it != c.end(); ++it )
    {
        visit_associative_container_key( it->first, it->second, v );
    }
}

} // namespace Impl {

/// vector visiting traits
template < typename T, typename A >
struct traits< std::vector< T, A > >
{
    /// visit
    template < typename K, typename V > static void visit( const K& key, std::vector< T, A >& t, V& v )
    {
        Impl::visit_non_associative_container( key, t, v );
    }
    
    /// visit const
    template < typename K, typename V > static void visit( const K& key, const std::vector< T, A >& t, V& v )
    {
        Impl::visit_non_associative_container( key, t, v );
    }    
};

/// set visiting traits
template < typename T >
struct traits< std::set< T > >
{
    /// visit
    template < typename K, typename V > static void visit( const K& key, std::set< T >& t, V& v )
    {
        Impl::visit_non_associative_container( key, t, v );
    }
    
    /// visit const
    template < typename K, typename V > static void visit( const K& key, const std::set< T >& t, V& v )
    {
        Impl::visit_non_associative_container( key, t, v );
    }    
};

/// boost array visiting traits
template < typename T, std::size_t S >
struct traits< boost::array< T, S > >
{
    /// visit
    template < typename K, typename V > static void visit( const K& key, boost::array< T, S >& t, V& v )
    {
        Impl::visit_non_associative_container( key, t, v );
    }
    
    /// visit const
    template < typename K, typename V > static void visit( const K& key, const boost::array< T, S >& t, V& v )
    {
        Impl::visit_non_associative_container( key, t, v );
    }    
};

/// set visiting traits
template < typename T, typename S >
struct traits< std::map< T, S > >
{
    /// visit
    template < typename K, typename V > static void visit( const K& key, std::map< T, S >& t, V& v )
    {
        Impl::visit_associative_container( key, t, v );
    }
    
    /// visit const
    template < typename K, typename V > static void visit( const K& key, const std::map< T, S >& t, V& v )
    {
        Impl::visit_associative_container( key, t, v );
    }    
};

/// @todo add more types as needed

} } // namespace comma { namespace visiting {

#endif // COMMA_VISITING_TRAITS_HEADER_GUARD_
