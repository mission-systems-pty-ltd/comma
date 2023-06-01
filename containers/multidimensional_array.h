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

        multidimensional_array( const index_type& size, const V& default_value = V() ): _size( size ), _data( _product( _size ), default_value ) {}

        V& operator()( const index_type& i ) { return _data[ _index( i ) ]; }

        const V& operator()( const index_type& i ) const { return _data[ _index( i ) ]; }

        const std::vector< V >& data() const { return _data; }

        const index_type& size() const { return _size; }

        class const_iterator;

        class iterator
        {
            public:
                iterator() = default;
                V& operator*() { return *_it; }
                const V& operator*() const { return *_it; }
                iterator& operator++() { ++_it; ++_i; return *this; }
                index_type index() const;
                bool operator==( const iterator& rhs ) const { return _it == rhs._it; }
                bool operator==( const const_iterator& rhs ) const { return _it == rhs._it; }
                bool operator!=( const iterator& rhs ) const { return !operator==( rhs ); }
                bool operator!=( const const_iterator& rhs ) const { return !operator==( rhs ); }

            private:
                friend class multidimensional_array< V, D >;
                std::size_t _i{0};
                typename std::vector< V >::iterator _it;
                index_type _size;
                iterator( std::size_t i, typename std::vector< V >::iterator it, const index_type& size ): _i( i ), _it( it ), _size( size ) {}
        };

        class const_iterator
        {
            public:
                const_iterator() = default;
                const V& operator*() const { return *_it; }
                const_iterator& operator++() { ++_it; ++_i; return *this; }
                index_type index() const;
                bool operator==( const iterator& rhs ) const { return _it == rhs._it; }
                bool operator==( const const_iterator& rhs ) const { return _it == rhs._it; }
                bool operator!=( const iterator& rhs ) const { return !operator==( rhs ); }
                bool operator!=( const const_iterator& rhs ) const { return !operator==( rhs ); }

            private:
                friend class multidimensional_array< V, D >;
                std::size_t _i{0};
                typename std::vector< V >::iterator _it;
                index_type _size;
                const_iterator( std::size_t i, typename std::vector< V >::iterator it, const index_type& size ): _i( i ), _it( it ), _size( size ) {}
        };

        iterator begin() { return iterator( 0, _data.begin(), _size ); }

        const_iterator begin() const { return const_iterator( 0, _data.begin(), _size ); }

        iterator end() { return iterator( _data.size(), _data.end(), _size ); }

        const_iterator end() const { return const_iterator( _data.size(), _data.end(), _size ); }

    private:
        index_type _size;
        std::vector< V > _data;
        std::size_t _index( const index_type& i );
        std::size_t _product( const index_type& i );
};



namespace impl {

template < unsigned int D, unsigned int I = D >
struct index
{
    typedef std::array< std::size_t, D > index_type;
    static unsigned int value( const index_type& i, const index_type& size ) { return i[ I - 1 ] + index< D, I - 1 >::value( i, size ) * size[ I - 1 ]; }
    static void value( std::size_t j, index_type& i, const index_type& size ) { i[ I - 1 ] = j % size[ I - 1 ]; index< D, I - 1 >::value( j / size[ I - 1 ], i, size ); }
    static index_type value( std::size_t j, const index_type& size ) { index_type i; value( j, i, size ); return i; }
    static std::size_t product( const index_type& i ) { return i[ I - 1 ] * index< D, I - 1 >::product( i ); }
};

template < unsigned int D >
struct index< D, 1 >
{
    typedef std::array< std::size_t, D > index_type;
    static unsigned int value( const index_type& i, const index_type& ) { return i[0]; }
    static void value( std::size_t j, index_type& i, const index_type& size ) { i[0] = j; }
    static std::size_t product( const index_type& i ) { return i[0]; }
};

} // namespace impl {

template < typename V, unsigned int D >
inline std::size_t multidimensional_array< V, D >::_index( const typename multidimensional_array< V, D >::index_type& i ) { return impl::index< D >::value( i, _size ); }

template < typename V, unsigned int D >
inline std::size_t multidimensional_array< V, D >::_product( const typename multidimensional_array< V, D >::index_type& i ) { return impl::index< D >::product( i ); }

template < typename V, unsigned int D >
inline typename multidimensional_array< V, D >::index_type multidimensional_array< V, D >::iterator::index() const { return impl::index< D >::value( _i, _size ); }

template < typename V, unsigned int D >
inline typename multidimensional_array< V, D >::index_type multidimensional_array< V, D >::const_iterator::index() const { return impl::index< D >::value( _i, _size ); }

} // namespace comma {
