// Copyright (c) 2023 Vsevolod Vlaskine

/// @author vsevolod vlaskine

#pragma once

#include <array>
#include "../base/types.h"
#include "impl/array_traits.h"

namespace comma {

template < typename V, unsigned int D >
class multidimensional_array
{
    public:
        typedef std::array< std::size_t, D > index_type;

        typedef V value_type;

        const unsigned int dimensions{D};

        multidimensional_array( const index_type& size, const V& default_value = V() ): _size( size ), _data( _index( _size ), default_value ) {}

        V& operator()( const index_type& i ) { return _data[ _index( i ) ]; }

        const V& operator()( const index_type& i ) const { return _data[ _index( i ) ]; }

        const std::vector< V >& data() const { return _data; }

        const index_type& size() const { return _size; }

    private:
        index_type _size;
        std::vector< V > _data;
        std::size_t _index( const index_type& i );
};

namespace impl {

template < unsigned int D, unsigned int I = D >
struct index
{
    typedef std::array< std::size_t, D > index_type;
    static unsigned int value( const index_type& i, const index_type& size ) { return i[ I - 1 ] + index< D, I - 1 >::value( i, size ) * size[ I - 1 ]; }
};

template < unsigned int D >
struct index< D, 1 >
{
    typedef std::array< std::size_t, D > index_type;
    static unsigned int value( const index_type& i, const index_type& ) { return i[0]; }
};

} // namespace impl {

template < typename V, unsigned int D >
inline std::size_t multidimensional_array< V, D >::_index( const typename multidimensional_array< V, D >::index_type& i ) { return impl::index< D >::value( i, _size ); }

} // namespace comma {
