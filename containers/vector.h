// Copyright (c) 2011 The University of Sydney

/// @author vsevolod vlaskine

#pragma once

#include <cmath>
#include <vector>
#include "../base/exception.h"
#include "../math/compare.h"

namespace comma {

/// vector on a range of discretized values
/// @todo quick and dirty; maybe there is a better way than wrapping std::vector
template < typename Key, typename T, typename Diff = Key >
struct regular_vector : public std::vector< T >
{
    struct index_type
    {
        Key begin;
        Diff step;

        index_type() {}
        index_type( const Key& begin, const Diff& step ) : begin( begin ), step( step ) {}
        int operator()( const Key& key ) const { return std::floor( ( key - begin ) / step ); }
    };

    regular_vector();

    regular_vector( const Key& begin, const Diff& step, std::size_t size = 0 ) : std::vector< T >( size ), index( begin, step ) {}

    regular_vector( const index_type& index, std::size_t size = 0 ) : std::vector< T >( size ), index( index ) {}

    T& operator()( const Key& key ) { return this->operator[]( index( key ) ); }

    const T& operator()( const Key& key ) const { return this->operator[]( index( key ) ); }

    index_type index;
};

} // namespace comma {
