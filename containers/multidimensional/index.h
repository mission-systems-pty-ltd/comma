// Copyright (c) 2024 Vsevolod Vlaskine

/// @author vsevolod vlaskine

#pragma once

#include <array>
#include <cstring>
#include "../../base/types.h"

namespace comma { namespace containers { namespace multidimensional {

template < unsigned int D >
struct index: public std::array< std::size_t, D >
{
    typedef std::array< std::size_t, D > base_t;

    template < typename... Args > index( Args... args ): base_t( { args... } ) {}

    bool operator<( const index& rhs ) const;

    bool operator==( const index& rhs ) const;

    bool operator!=( const index& rhs ) const { return !operator==( rhs ); }

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

template < unsigned int D > inline bool index< D >::operator<( const index& rhs ) const // todo: unravel in compile time (compiler probably will do it anyway)
{
    for( unsigned int i = 0; i < D; ++i )
    {
        if( ( *this )[i] < rhs[i] ) { return true; }
    }
    return false;
}

template < unsigned int D > inline bool index< D >::operator==( const index& rhs ) const // todo: unravel in compile time (compiler probably will do it anyway)
{
    return std::memcmp( reinterpret_cast< const char* >( this ), reinterpret_cast< const char* >( &rhs ), sizeof( std::size_t ) * D ) == 0;
}

template < unsigned int D > inline index< D >& index< D >::increment( const index< D >& sizes ) // todo: unravel in compile time (compiler probably will do it anyway)
{
    for( unsigned int i{0}, j{D - 1}; i < D; ++i, --j )
    {
        if( ++( *this )[j] < sizes[j] ) { return *this; }
        ( *this )[j] = 0;
    }
    return *this;
}

} } } // namespace comma { namespace containers { namespace multidimensional {
