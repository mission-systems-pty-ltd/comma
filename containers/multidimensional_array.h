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

        class const_iterator;

        class iterator
        {
            public:
                iterator() = default;
                std::pair< index_type, V& >&& operator->();
                std::pair< index_type, const V& >&& operator->() const;
                V& operator*() { return *_it; }
                const V& operator*() const { return *_it; }
                iterator& operator++() { ++_it; ++_i; return *this; }
                bool operator==( const iterator& rhs ) const { return _it == rhs._it; }
                bool operator==( const const_iterator& rhs ) const { return _it == rhs._it; }
                bool operator!=( const iterator& rhs ) const { return !operator==( rhs ); }
                bool operator!=( const const_iterator& rhs ) const { return !operator==( rhs ); }

            private:
                friend class multidimensional_array< V, D >;
                std::size_t _i{0};
                typename std::vector< V >::iterator _it;
                iterator( std::size_t i, typename std::vector< V >::iterator it ): _i( i ), _it( it ) {}
        };

        class const_iterator
        {
            public:
                const_iterator() = default;
                std::pair< index_type, const V& >&& operator->() const;
                const V& operator*() const { return *_it; }
                const_iterator& operator++() { ++_it; ++_i; return *this; }
                bool operator==( const iterator& rhs ) const { return _it == rhs._it; }
                bool operator==( const const_iterator& rhs ) const { return _it == rhs._it; }
                bool operator!=( const iterator& rhs ) const { return !operator==( rhs ); }
                bool operator!=( const const_iterator& rhs ) const { return !operator==( rhs ); }

            private:
                friend class multidimensional_array< V, D >;
                std::size_t _i{0};
                typename std::vector< V >::iterator _it;
                const_iterator( std::size_t i, typename std::vector< V >::iterator it ): _i( i ), _it( it ) {}
        };

        iterator begin() { return iterator( 0, _data.begin() ); }

        const_iterator begin() const { return const_iterator( 0, _data.begin() ); }

        iterator end() { return iterator( _data.size(), _data.end() ); }

        const_iterator end() const { return const_iterator( _data.size(), _data.end() ); }

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
    static void value( std::size_t j, index_type& i, const index_type& size ) { i[ I - 1 ] = j % size[ I - 1 ]; index< D, I - 1 >::value( j / size[ I - 1 ], i, size ); }
    static index_type value( std::size_t j, const index_type& size ) { index_type i; value( j, i, size ); return i; }
};

template < unsigned int D >
struct index< D, 1 >
{
    typedef std::array< std::size_t, D > index_type;
    static unsigned int value( const index_type& i, const index_type& ) { return i[0]; }
    static void value( std::size_t j, index_type& i, const index_type& size ) { i[0] = j; }
};

} // namespace impl {

template < typename V, unsigned int D >
inline std::size_t multidimensional_array< V, D >::_index( const typename multidimensional_array< V, D >::index_type& i ) { return impl::index< D >::value( i, _size ); }

template < typename V, unsigned int D >
std::pair< typename multidimensional_array< V, D >::index_type, V& >&& multidimensional_array< V, D >::iterator::operator->() { return std::make_pair( impl::index< D >::value( _i, _size ), *_it ); }

template < typename V, unsigned int D >
std::pair< typename multidimensional_array< V, D >::index_type, const V& >&& multidimensional_array< V, D >::iterator::operator->() const { return std::make_pair( impl::index< D >::value( _i, _size ), *_it ); }

template < typename V, unsigned int D >
std::pair< typename multidimensional_array< V, D >::index_type, const V& >&& multidimensional_array< V, D >::const_iterator::operator->() const { return std::make_pair( impl::index< D >::value( _i, _size ), *_it ); }

} // namespace comma {
