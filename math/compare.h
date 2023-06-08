// Copyright (c) 2011 The University of Sydney

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
