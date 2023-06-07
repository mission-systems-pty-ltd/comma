// Copyright (c) 2023 Vsevolod Vlaskine

/// @author vsevolod vlaskine

#pragma once

#include <array>
#include <unordered_map>
#include <boost/functional/hash.hpp>
#include "../../base/types.h"
#include "array_traits.h"

namespace comma { namespace containers { namespace multidimensional {

/// quick and dirty hash for array-like containers (its support is awkward in boost)
template < typename Array, std::size_t Size >
struct array_hash : public std::unary_function< Array, std::size_t >
{
    std::size_t operator()( Array const& array ) const
    {
        std::size_t seed = 0;
        for( std::size_t i = 0; i < Size; ++i ) { boost::hash_combine( seed, array[i] ); }
        return seed;
        // return boost::hash_range( &array[0], &array[Size] ); // not so easy...
    }
};

/// unordered map with array-like keys
template < typename K, typename V, unsigned int Size, typename P = std::array< K, Size >, typename Traits = impl::operations< Size > >
class map : public std::unordered_map< std::array< comma::int32, Size >, V, array_hash< std::array< comma::int32, Size >, Size > >
{
    public:
        typedef std::unordered_map< std::array< comma::int32, Size >, V, array_hash< std::array< comma::int32, Size >, Size > > base_type;

        typedef base_type map_type;

        typedef base_type as_map;
        
        enum { dimensions = Size };
        
        typedef P point_type;

        typedef typename base_type::key_type index_type;

        typedef typename base_type::key_type key_type; // for brevity

        typedef typename base_type::mapped_type mapped_type; // for brevity
        
        typedef typename base_type::iterator iterator; // otherwise it does not build on windows...
        
        typedef typename base_type::const_iterator const_iterator; // otherwise it does not build on windows...

        /// constructor
        map( const point_type& origin, const point_type& resolution );

        /// constructor, origin is all zeroes
        map( const point_type& resolution );
        
        /// insert element at the given point, if it does not exist
        iterator touch_at( const point_type& point );
        
        /// insert element at the given point, if it does not exist
        std::pair< iterator, bool > insert( const point_type& point, const mapped_type& value );
        
        /// return index of the point, always rounds it down (does floor for a given resolution)
        key_type index_of( const point_type& point ) const;
        
        /// same as index_of( point ), but static
        static key_type index_of( const point_type& point, const point_type& origin, const point_type& resolution );
        
        /// same as index_of( point ), but static with origin assumed all zeroes
        static key_type index_of( const point_type& point, const point_type& resolution );
        
        /// find value by point
        iterator find( const point_type& point );
        
        /// find value by point
        const_iterator find( const point_type& point ) const;
        
        /// find value by key
        iterator find( const key_type& index );
        
        /// find voxel by key
        const_iterator find( const key_type& index ) const;
        
        /// return origin
        const point_type& origin() const;
        
        /// return resolution
        const point_type& resolution() const;

    private:
        point_type _origin;
        point_type _resolution;
};

template < typename K, typename V, unsigned int Size, typename P, typename Traits >
inline map< K, V, Size, P, Traits >::map( const typename map< K, V, Size, P, Traits >::point_type& origin, const typename map< K, V, Size, P, Traits >::point_type& resolution )
    : _origin( origin )
    , _resolution( resolution )
{
}

template < typename K, typename V, unsigned int Size, typename P, typename Traits >
inline map< K, V, Size, P, Traits >::map( const typename map< K, V, Size, P, Traits >::point_type& resolution )
    : _origin( Traits::template zero< P >() )
    , _resolution( resolution )
{
}

template < typename K, typename V, unsigned int Size, typename P, typename Traits >
inline typename map< K, V, Size, P, Traits >::iterator map< K, V, Size, P, Traits >::touch_at( const typename map< K, V, Size, P, Traits >::point_type& point )
{
    key_type index = index_of( point );
    iterator it = this->base_type::find( index );
    if( it != this->end() ) { return it; }
    return this->base_type::insert( std::make_pair( index, mapped_type() ) ).first;
}

template < typename K, typename V, unsigned int Size, typename P, typename Traits >
inline std::pair< typename map< K, V, Size, P, Traits >::iterator, bool > map< K, V, Size, P, Traits >::insert( const typename map< K, V, Size, P, Traits >::point_type& point, const typename map< K, V, Size, P, Traits >::mapped_type& value )
{
    return this->base_type::insert( std::make_pair( index_of( point ), value ) );
}

template < typename K, typename V, unsigned int Size, typename P, typename Traits >
inline typename map< K, V, Size, P, Traits >::key_type map< K, V, Size, P, Traits >::index_of( const typename map< K, V, Size, P, Traits >::point_type& point, const typename map< K, V, Size, P, Traits >::point_type& origin, const typename map< K, V, Size, P, Traits >::point_type& resolution )
{
    return Traits::template index_of< P, key_type >( point, origin, resolution );
}

template < typename K, typename V, unsigned int Size, typename P, typename Traits >
inline typename map< K, V, Size, P, Traits >::key_type map< K, V, Size, P, Traits >::index_of( const typename map< K, V, Size, P, Traits >::point_type& point, const typename map< K, V, Size, P, Traits >::point_type& resolution )
{
    return index_of( point, Traits::template zero< P >(), resolution );
}

template < typename K, typename V, unsigned int Size, typename P, typename Traits >
inline typename map< K, V, Size, P, Traits >::key_type map< K, V, Size, P, Traits >::index_of( const typename map< K, V, Size, P, Traits >::point_type& point ) const
{
    return index_of( point, _origin, _resolution );
}

template < typename K, typename V, unsigned int Size, typename P, typename Traits >
inline typename map< K, V, Size, P, Traits >::iterator map< K, V, Size, P, Traits >::find( const typename map< K, V, Size, P, Traits >::point_type& point )
{
    index_type i = index_of( point );
    return this->base_type::find( i );
}

template < typename K, typename V, unsigned int Size, typename P, typename Traits >
inline typename map< K, V, Size, P, Traits >::const_iterator map< K, V, Size, P, Traits >::find( const typename map< K, V, Size, P, Traits >::point_type& point ) const
{
    index_type i = index_of( point );
    return this->base_type::find( i );
}

template < typename K, typename V, unsigned int Size, typename P, typename Traits >
inline typename map< K, V, Size, P, Traits >::iterator map< K, V, Size, P, Traits >::find( const typename map< K, V, Size, P, Traits >::key_type& index )
{
    return this->base_type::find( index ); // otherwise strange things happen... debug, when we have time
}

template < typename K, typename V, unsigned int Size, typename P, typename Traits >
inline typename map< K, V, Size, P, Traits >::const_iterator map< K, V, Size, P, Traits >::find( const typename map< K, V, Size, P, Traits >::key_type& index ) const
{
    return this->base_type::find( index ); // otherwise strange things happen... debug, when we have time
}

template < typename K, typename V, unsigned int Size, typename P, typename Traits >
inline const typename map< K, V, Size, P, Traits >::point_type& map< K, V, Size, P, Traits >::origin() const { return _origin; }

template < typename K, typename V, unsigned int Size, typename P, typename Traits >
inline const typename map< K, V, Size, P, Traits >::point_type& map< K, V, Size, P, Traits >::resolution() const { return _resolution; }

} } } // namespace comma { namespace containers { namespace multidimensional {
