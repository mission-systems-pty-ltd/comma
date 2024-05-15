// Copyright (c) 2011 The University of Sydney
// Copyright (c) 2022 Vsevolod Vlaskine

/// @author vsevolod vlaskine

#pragma once

#ifndef WIN32
#include <stdlib.h>
#endif
#include <array>
#include <complex>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <boost/array.hpp>
#include "../base/exception.h"
#include "../base/none.h"
#include "../base/optional.h"
#include "../base/variant.h"

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

template < typename T >
struct traits< comma::optional< T > >
{
    template < typename K, typename V > static void visit( const K& key, comma::optional< T >& t, V& v )
    {
        v.apply( "value", t.value );
        v.apply( "is_set", t.is_set );
    }
    
    template < typename K, typename V > static void visit( const K&, const comma::optional< T >& t, V& v )
    {
        v.apply( "value", t.value );
        v.apply( "is_set", t.is_set );
    }    
};

template < typename... Args >
struct traits< comma::variant< Args... > > // todo? should it be in the visitors instead? todo!
{
    typedef comma::variant< Args... > variant_t;

    template < typename S, Args... > struct _variant_traits // todo
    {
        template < typename K, typename V > void _visit( const K& key, variant_t& t, V& v, bool is_set )
        {
            boost::optional< S > s = t.template optional< S >();
            traits< boost::optional< S > >::visit( key, s, v );
            COMMA_ASSERT( !s || !is_set, "variant: ambiguous: expected not more than one variant type set; got at least two variant types set" );
            if( s ) { t.set( s ); }
            _variant_traits< Args... >::_visit( key, t, v, bool( s ) );
        }
        template < typename K, typename V > void _visit( const K& key, const variant_t& t, V& v )
        {
            const boost::optional< S >& s = t.template optional< S >();
            if( s ) { traits< S >::visit( key, *s, v ); } else { _variant_traits< Args... >::_visit( key, t, v ); }
        }
    };

    template < typename K, typename V > static void visit( const K& key, variant_t& t, V& v )
    {
        _variant_traits< Args... >::visit( key, t, v, false );
    }
    
    template < typename K, typename V > static void visit( const K& key, const variant_t& t, V& v )
    {
        _variant_traits< Args... >::visit( key, t, v );
    }
};

template < typename T >
struct traits< comma::variant< T > >
{
    typedef comma::variant< T > variant_t;

    template < typename S > struct _variant_traits // todo
    {
        template < typename K, typename V > void _visit( const K& key, variant_t& t, V& v, bool is_set )
        {
            boost::optional< S > s = t.template optional< S >();
            traits< boost::optional< S > >::visit( key, s, v );
            COMMA_ASSERT( !s || !is_set, "variant: ambiguous: expected not more than one variant type set; got at least two variant types set" );
            if( s ) { t.set( s ); }
        }
        template < typename K, typename V > void _visit( const K& key, const variant_t& t, V& v )
        {
            const boost::optional< S >& s = t.template optional< S >();
            if( s ) { traits< S >::visit( key, *s, v ); }
        }
    };

    template < typename K, typename V > static void visit( const K& key, variant_t& t, V& v )
    {
        _variant_traits< T >::visit( key, t, v, false );
    }
    
    template < typename K, typename V > static void visit( const K& key, const variant_t& t, V& v )
    {
        _variant_traits< T >::visit( key, t, v );
    }
};

template < typename Names, typename... NArgs >
struct traits< comma::named_variant< Names, NArgs... > > // todo? should it be in the visitors instead?
{
    typedef comma::named_variant< Names, NArgs... > named_variant_t;
    typedef comma::variant< NArgs... > variant_t;

    template < typename S, typename... Args > struct _variant_traits // todo
    {
        template < typename V > static void visit( variant_t& t, V& v, unsigned int i, bool is_set )
        {
            boost::optional< S > s = t.template optional< S >();
            v.apply( named_variant_t::names()[i], s );
            COMMA_ASSERT( !s || !is_set, "variant: ambiguous: expected not more than one variant type set; got at least two variant types set" );
            if( s ) { t.set( s ); }
            _variant_traits< Args... >::visit( t, v, ++i, bool( s ) );
        }
        template < typename V > static void visit( const variant_t& t, V& v, unsigned int i )
        {
            const boost::optional< S >& s = t.template optional< S >();
            if( s ) { v.apply( named_variant_t::names()[i], *s ); } else { _variant_traits< Args... >::visit( t, v, ++i ); }
        }
    };

    template < typename S > struct _variant_traits< S > // todo
    {
        template < typename V > static void visit( variant_t& t, V& v, bool is_set, unsigned int i )
        {
            boost::optional< S > s = t.template optional< S >();
            v.apply( named_variant_t::names()[i], s );
            COMMA_ASSERT( !s || !is_set, "variant: ambiguous: expected not more than one variant type set; got at least two variant types set" );
            if( s ) { t.set( s ); }
        }
        template < typename V > static void visit( const variant_t& t, V& v, unsigned int i )
        {
            const boost::optional< S >& s = t.template optional< S >();
            if( s ) { v.apply( named_variant_t::names()[i], *s ); }
        }
    };

    template < typename K, typename V > static void visit( const K&, named_variant_t& t, V& v )
    {
        _variant_traits< NArgs... >::visit( static_cast< variant_t& >( t ), v, 0, false );
    }
    
    template < typename K, typename V > static void visit( const K& key, const named_variant_t& t, V& v )
    {
        _variant_traits< NArgs... >::visit( static_cast< const variant_t& >( t ), v, 0 );
    }
};

// template < typename Names, typename T >
// struct traits< comma::named_variant< T > >
// {
//     typedef typename comma::variant< T >::variant_t variant_t;

//     template < typename S > struct _variant_traits // todo
//     {
//         template < typename K, typename V > void _visit( const K& key, variant_t& t, V& v, bool is_set, unsigned int i )
//         {
//             boost::optional< S > s = t.template optional< S >();
//             v.apply( variant_t::names()[i], s );
//             COMMA_ASSERT( !s || !is_set, "variant: ambiguous: expected not more than one variant type set; got at least two variant types set" );
//             if( s ) { t.set( s ); }
//         }
//         template < typename K, typename V > void _visit( const K& key, const variant_t& t, V& v, unsigned int i )
//         {
//             const boost::optional< S >& s = t.template optional< S >();
//             if( s ) { v.apply( variant_t::names()[i], *s ); }
//         }
//     };

//     template < typename K, typename V > static void visit( const K& key, variant_t& t, V& v )
//     {
//         _variant_traits< T >::visit( key, t, v, false );
//     }
    
//     template < typename K, typename V > static void visit( const K& key, const variant_t& t, V& v )
//     {
//         _variant_traits< T >::visit( key, t, v );
//     }
// };

template < typename T >
struct traits< std::complex< T > >
{
    template < typename K, typename V > static void visit( const K& key, std::complex< T >& t, V& v )
    {
        T s = t.real();
        v.apply( "real", s );
        t.real( s );
        s = t.imag();
        v.apply( "imag", s );
        t.imag( s );
    }
    
    template < typename K, typename V > static void visit( const K& key, const std::complex< T >& t, V& v )
    {
        v.apply( "real", t.real() );
        v.apply( "imag", t.imag() );
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
