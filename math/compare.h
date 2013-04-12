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
// 3. All advertising materials mentioning features or use of this software
//    must display the following acknowledgement:
//    This product includes software developed by the The University of Sydney.
// 4. Neither the name of the The University of Sydney nor the
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

#ifndef COMMA_MATH_COMPARE_H_
#define COMMA_MATH_COMPARE_H_

#include <cmath>
#include <limits>
#include <algorithm>

namespace comma { namespace math {

template < typename S > inline bool equal( float lhs, S rhs ) { return std::fabs( lhs - rhs ) < std::numeric_limits< float >::epsilon(); }
template < typename S > inline bool less( float lhs, S rhs ) { return static_cast< float >( rhs ) - lhs >= std::numeric_limits< float >::epsilon(); }

template < typename S > inline bool equal( double lhs, S rhs ) { return std::fabs( lhs - rhs ) < std::numeric_limits< double >::epsilon(); }
template < typename S > inline bool less( double lhs, S rhs ) { return static_cast< double >( rhs ) - lhs >= std::numeric_limits< double >::epsilon(); }

template < typename S > inline bool equal( long double lhs, S rhs ) { return std::fabs( lhs - rhs ) < std::numeric_limits< long double >::epsilon(); }
template < typename S > inline bool less( long double lhs, S rhs ) { return static_cast< long double >( rhs ) - lhs >= std::numeric_limits< long double >::epsilon(); }

template < typename T, typename S > inline bool equal( const T& lhs, const S& rhs ) { return lhs == rhs; }
template < typename T, typename S > inline bool less( const T& lhs, const S& rhs ) { return lhs < rhs; }

template < typename T, typename S, typename Diff > inline bool less( const T& lhs, const S& rhs, const Diff& epsilon ) { return less( lhs + epsilon, rhs ); }
template < typename T, typename S, typename Diff > inline bool equal( const T& lhs, const S& rhs, const Diff& epsilon ) { return !less( lhs, rhs, epsilon ) && !less( static_cast< T >( rhs ), lhs, epsilon ); }

template < typename T, typename S > inline T min( const T& rhs, const S& lhs ) { return std::min( rhs, lhs ); }
template < typename T, typename S > inline T max( const T& rhs, const S& lhs ) { return std::max( rhs, lhs ); }


} } // namespace comma { namespace math {

#endif // COMMA_MATH_COMPARE_H_
