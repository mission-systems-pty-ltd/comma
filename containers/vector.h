// This file is part of comma, a generic and flexible library
// Copyright (c) 2011 The University of Sydney
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. Neither the name of the University of Sydney nor the
//    names of its contributors may be used to endorse or promote products
//    derived from this software without specific prior written permission.
//
// NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
// GRANTED BY THIS LICENSE.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
// HOLDERS AND CONTRIBUTORS \"AS IS\" AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
// BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
// IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


/// @author vsevolod vlaskine

#ifndef COMMA_CONTAINERS_VECTOR_H_
#define COMMA_CONTAINERS_VECTOR_H_

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

#endif // COMMA_CONTAINERS_VECTOR_H_
