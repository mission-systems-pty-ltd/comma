// Copyright (c) 2023 Vsevolod Vlaskine

/// @author vsevolod vlaskine

#pragma once

#include <array>
#include "../../base/types.h"
#include "array_traits.h"

namespace comma { namespace containers { namespace multidimensional {

template < typename V, unsigned int D >
class slice
{
    public:
        typedef std::array< std::size_t, D > index_type;

        typedef V value_type;

        static const unsigned int dimensions{D};

        slice( const index_type& shape, V* data ): _shape( shape ), _size( _product( _shape ) ), _data( data ) {}

        slice& operator=( const slice& rhs ) = default;

        V& operator[]( const index_type& i ) { return _data[ _index( i ) ]; }

        const V& operator[]( const index_type& i ) const { return _data[ _index( i ) ]; }

        template < unsigned int I >
        slice< V, D - I > at( const std::array< std::size_t, I >& i );

        template < unsigned int I >
        const slice< V, D - I > at( const std::array< std::size_t, I >& i ) const;

        slice< V, D - 1 > at( std::size_t i ) { return at< 1 >( std::array< std::size_t, 1 >{i} ); }

        const slice< V, D - 1 > at( std::size_t i ) const { return at< 1 >( std::array< std::size_t, 1 >{i} ); }

        V* data() { return _data; }
        
        const V* data() const { return _data; }

        const index_type& shape() const { return _shape; }

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
                friend class slice< V, D >;
                std::size_t _i{0};
                V* _it{nullptr};
                index_type _shape;
                iterator( std::size_t i, V* it, const index_type& shape ): _i( i ), _it( it ), _shape( shape ) {}
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
                friend class slice< V, D >;
                std::size_t _i{0};
                const V* _it{nullptr};
                index_type _shape;
                const_iterator( std::size_t i, V* it, const index_type& shape ): _i( i ), _it( it ), _shape( shape ) {}
        };

        iterator begin() { return iterator( 0, _data, _shape ); }

        const_iterator begin() const { return const_iterator( 0, _data, _shape ); }

        iterator end() { return iterator( _size, _data + _size, _shape ); }

        const_iterator end() const { return const_iterator( _size, _data + _size, _shape ); }

    private:
        index_type _shape;
        std::size_t _size;
        V* _data;
        std::size_t _index( const index_type& i ) const;
        static std::size_t _product( const index_type& i );
};


template < typename V, unsigned int D, typename S = std::vector< V > >
class array
{
    public:
        typedef slice< V, D > slice_type;

        typedef typename slice_type::index_type index_type;

        typedef V value_type;

        typedef S storage_type;

        const unsigned int dimensions{D};

        array( const index_type& shape, const V& default_value = V() );

        V& operator[]( const index_type& i ) { return _slice[i]; }

        const V& operator[]( const index_type& i ) const { return _slice[i]; }

        template < unsigned int I >
        multidimensional::slice< V, D - I > at( const std::array< std::size_t, I >& i ) { return _slice.template at< I >( i ); }

        template < unsigned int I >
        const multidimensional::slice< V, D - I > at( const std::array< std::size_t, I >& i ) const { return _slice.template at< I >( i ); }

        multidimensional::slice< V, D - 1 > at( std::size_t i ) { return _slice.at( i ); }

        const multidimensional::slice< V, D - 1 > at( std::size_t i ) const { return _slice.at( i ); }

        storage_type& data() { return _data; }

        const storage_type& data() const { return _data; }

        const index_type& shape() const { return _slice.shape(); }

        typedef typename slice_type::iterator iterator;

        typedef typename slice_type::const_iterator const_iterator;

        iterator begin() { return _slice.begin(); }

        const_iterator begin() const { return _slice.begin(); }

        iterator end() { return _slice.end(); }

        const_iterator end() const { return _slice.end(); }

    private:
        storage_type _data;
        slice_type _slice;
};

template < typename V, unsigned int D, typename P = std::array< double, D >, typename Traits = impl::operations< D >, typename S = std::vector< V > >
class grid: public array< V, D, S >
{
    public:
        typedef P point_type;

        typedef array< V, D, S > base_type;

        typedef typename base_type::index_type index_type;

        typedef typename base_type::value_type value_type;

        grid( const P& origin, const P& resolution, const index_type& shape, const V& default_value = V() ): base_type( shape, default_value ), _origin( origin ), _resolution( resolution ) {}

        index_type index_of( const point_type& point ) const { return Traits::template index_of< P, index_type >( point, _origin, _resolution ); }

        point_type lower_bound( const point_type& point ) const { return Traits::add( _origin + Traits::multiply( _resolution, index_of( point ) ) ); }

        V& operator()( const point_type& p ) { return this->operator[]( index_of( p ) ); }

        const V& operator()( const point_type& p ) const { return this->operator[]( index_of( p ) ); }

        const point_type& origin() const { return _origin; }

        const point_type& resolution() const { return _resolution; }

        V interpolated( const point_type& point ) const; // todo: flag/enum or alike for different interpolation types; currently linear only

    private:
        point_type _origin;
        point_type _resolution;
};

namespace impl {

template < unsigned int D, unsigned int I = D >
struct index_traits
{
    typedef std::array< std::size_t, D > index_type;
    static unsigned int value( const index_type& i, const index_type& shape ) { return i[ I - 1 ] + index_traits< D, I - 1 >::value( i, shape ) * shape[ I - 1 ]; }
    static void value( std::size_t j, index_type& i, const index_type& shape ) { i[ I - 1 ] = j % shape[ I - 1 ]; index_traits< D, I - 1 >::value( j / shape[ I - 1 ], i, shape ); }
    static index_type value( std::size_t j, const index_type& shape ) { index_type i; value( j, i, shape ); return i; }
    static std::size_t product( const index_type& i ) { return i[ I - 1 ] * index_traits< D, I - 1 >::product( i ); }
    template < unsigned int J >
    static std::pair< std::array< std::size_t, J >, std::array< std::size_t, D - J > > split( const index_type& i ) // todo: use metaprogramming, kinda same as product
    {
        std::pair< std::array< std::size_t, J >, std::array< std::size_t, D - J > > p;
        unsigned int k = 0;
        for( unsigned int n = 0; n < J; ++n, ++k ) { p.first[n] = i[k]; }
        for( unsigned int n = 0; n < D - J; ++n, ++k ) { p.second[n] = i[k]; }
        return p;
    }
};

template < unsigned int D >
struct index_traits< D, 1 >
{
    typedef std::array< std::size_t, D > index_type;
    static unsigned int value( const index_type& i, const index_type& ) { return i[0]; }
    static void value( std::size_t j, index_type& i, const index_type& size ) { i[0] = j; }
    static std::size_t product( const index_type& i ) { return i[0]; }
};

} // namespace impl {

template < typename V, unsigned int D >
inline std::size_t slice< V, D >::_index( const typename slice< V, D >::index_type& i ) const { return impl::index_traits< D >::value( i, _shape ); }

template < typename V, unsigned int D >
inline std::size_t slice< V, D >::_product( const typename slice< V, D >::index_type& i ) { return impl::index_traits< D >::product( i ); }

template < typename V, unsigned int D >
inline typename slice< V, D >::index_type slice< V, D >::iterator::index() const { return impl::index_traits< D >::value( _i, _shape ); }

template < typename V, unsigned int D >
inline typename slice< V, D >::index_type slice< V, D >::const_iterator::index() const { return impl::index_traits< D >::value( _i, _shape ); }

template < typename V, unsigned int D >
template < unsigned int I >
inline slice< V, D - I > slice< V, D >::at( const std::array< std::size_t, I >& i )
{
    auto s = impl::index_traits< D >::template split< I >( _shape );
    return slice< V, D - I >( s.second, _data + impl::index_traits< I >::value( i, s.first ) * impl::index_traits< D - I >::product( s.second ) );
}

template < typename V, unsigned int D >
template < unsigned int I >
inline const slice< V, D - I > slice< V, D >::at( const std::array< std::size_t, I >& i ) const
{
    auto s = impl::index_traits< D >::template split< I >( _shape );
    return slice< V, D - I >( s.second, _data + impl::index_traits< I >::value( i, s.first ) * impl::index_traits< D - I >::product( s.second ) );
}

template < typename V, unsigned int D, typename S >
inline array< V, D, S >::array( const typename array< V, D, S >::index_type& shape, const V& default_value ): _data( impl::index_traits< D >::product( shape ), default_value ), _slice( shape, &_data[0] ) {}

template < typename V, unsigned int D, typename P, typename Traits, typename S >
V grid< V, D, P, Traits, S >::interpolated( const P& point ) const
{
    P element_origin = _resolution;
    const index_type i = index_of( point );
    Traits::add( Traits::vmultiply( element_origin, i ), _origin );
    const auto& weights = Traits::interpolation::linear::weights( point, element_origin, _resolution );
    const auto& neighbours = impl::neighbours< index_type, D >;
    V v = this->operator[]( i ) * weights[0]; // todo?! value traits?!
    for( unsigned int j = 1; j < weights.size(); ++j ) { v += this->operator[]( Traits::add( i, neighbours[j] ) ) * weights[j]; } // todo?! value traits?!
    return v;
}

} } } // namespace comma { namespace containers { namespace multidimensional {
