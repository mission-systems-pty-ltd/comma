// Copyright (c) 2024 Vsevolod Vlaskine

/// @author vsevolod vlaskine

#pragma once

#include <array>
#include <cstring>
#include "../../base/types.h"

namespace comma { namespace containers { namespace multidimensional {

template < unsigned int D, typename T = std::size_t >
struct index: public std::array< T, D >
{
    typedef T value_t;

    typedef std::array< T, D > base_t;

    index(): base_t{} {}

    index( const index& rhs ): base_t( static_cast< const base_t& >( rhs ) ) {}

    template < typename... Args > index( T t, Args... args ); // quick and dirty for now to avoid compile warning // template < typename... Args > index( Args... args ): base_t( { args... } ) {}

    bool operator<( const index& rhs ) const;

    bool operator==( const index& rhs ) const;

    bool operator!=( const index& rhs ) const { return !operator==( rhs ); }

    index& operator=( const index& rhs ) = default;

    index& operator=( const base_t& rhs ) { static_cast< base_t& >( *this ) = rhs; }

    operator base_t() { return static_cast< base_t& >( *this ); }

    operator base_t() const { return static_cast< const base_t& >( *this ); }

    index& increment( const index& sizes );

    class iterator
    {
        public:
            iterator( const index< D >& shape ): _shape( shape ) {}

            iterator& operator++() { _valid = _index.increment( _shape ) != index< D >{}; return *this; }

            operator bool() const { return _valid; }
            
            const index< D >& operator*() const { return _index; }

        private:
            index< D > _index;
            index< D > _shape;
            bool _valid{true};
    };
};

namespace impl {

template < typename T, unsigned int D, unsigned int I > struct type_cast
{
    template < typename... Args > static void assign( index< D, T >& i, T v, Args... args )
    {
        i[ D - I ] = v;
        type_cast< T, D, I - 1 >::assign( i, args... );
    }
};

template < typename T, unsigned int D > struct type_cast< T, D, 1 >
{
    static void assign( index< D, T >& i, T v ) { i[ D - 1 ] = v; }
};

} // namespace impl {

template < unsigned int D, typename T > 
template < typename... Args > inline index< D, T >::index( T t, Args... args )
{
    impl::type_cast< T, D, D >::assign( *this, t, args... );
}

template < unsigned int D, typename T > inline bool index< D, T >::operator<( const index& rhs ) const // todo: unravel in compile time (compiler probably will do it anyway)
{
    for( unsigned int i = 0; i < D; ++i )
    {
        if( ( *this )[i] < rhs[i] ) { return true; }
    }
    return false;
}

template < unsigned int D, typename T > inline bool index< D, T >::operator==( const index& rhs ) const // todo: unravel in compile time (compiler probably will do it anyway)
{
    return std::memcmp( reinterpret_cast< const char* >( this ), reinterpret_cast< const char* >( &rhs ), sizeof( std::size_t ) * D ) == 0;
}

template < unsigned int D, typename T > inline index< D, T >& index< D, T >::increment( const index< D, T >& sizes ) // todo: unravel in compile time (compiler probably will do it anyway)
{
    for( unsigned int i{0}, j{D - 1}; i < D; ++i, --j )
    {
        if( ++( *this )[j] < sizes[j] ) { return *this; }
        ( *this )[j] = 0;
    }
    return *this;
}

} } } // namespace comma { namespace containers { namespace multidimensional {
