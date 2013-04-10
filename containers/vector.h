// This file is part of comma, a generic and flexible library
// for robotics research.
//
// Copyright (C) 2011 The University of Sydney
//
// comma is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// comma is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
// for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with comma. If not, see <http://www.gnu.org/licenses/>.

/// @author vsevolod vlaskine

#ifndef COMMA_CONTAINERS_VECTOR_H_
#define COMMA_CONTAINERS_VECTOR_H_

#include <cmath>
#include <vector>
#include <comma/base/exception.h>
#include <comma/math/compare.h>

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

    T& operator()( const Key& key ) { return this->operator[]( index() ); }

    const T& operator()( const Key& key ) const { return this->operator[]( index() ); }

    index_type index;
};

} // namespace comma {

#endif // COMMA_CONTAINERS_VECTOR_H_
