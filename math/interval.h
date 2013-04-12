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

#ifndef COMMA_MATH_INTERVAL_H_
#define COMMA_MATH_INTERVAL_H_

#include <iostream>
#include <comma/base/exception.h>
#include <comma/math/compare.h>

namespace comma { namespace math {

/// [x, y) interval
template < typename T >
class interval
{
    public:
        /// copy constructor
        interval( const interval& rhs ) { this->operator=( rhs ); } 
        
        /// constructor
        interval( const T& min, const T& max ) : interval_( min, max ) { if( !math::less( min, max ) ) { COMMA_THROW( comma::exception, "invalid interval" ); } }
        
        /// return value
        const std::pair< T, T >& operator()() const { return interval_; }
        
        /// return left boundary (convenience method)
        T min() const { return interval_.first; }
        
        /// return right boundary (convenience method)
        T max() const { return interval_.second; }
                
        /// return interval size (convenience method)
        T size() const { return interval_.second - interval_.first; }
        
        /// return true, if variable belongs to the interval
        bool contains( const T& t ) const { return !math::less( t, interval_.first ) && math::less( t, interval_.second ); }
        
        /// return true, if variable belongs to the interval
        bool contains( const interval& rhs ) const { return !( math::less( rhs().first, interval_.first ) || math::less( interval_.second, rhs().second ) ); }

        /// compute the hull of the interval and [x]
        interval< T > hull( const T& x ) { return interval( std::min( interval_.first, x ), std::max( interval_.second, x ) ); }

        /// compute the hull of 2 intervals
        interval< T > hull( const interval& rhs ) { return interval( std::min( interval_.first, rhs.min() ), std::max( interval_.second, rhs.max() ) ); }
        
        /// equality
        bool operator==( const interval& rhs ) const { return math::equal( interval_.first, rhs().first ) && math::equal( interval_.second, rhs().second ); }
        
        /// unequality
        bool operator!=( const interval& rhs ) const { return !operator==( rhs ); }
        
    private:
        std::pair< T, T > interval_;
};

/// [x, y] interval
template < typename T >
class closed_interval
{
    public:
        /// copy constructor
        closed_interval( const closed_interval& rhs ) { this->operator=( rhs ); } 
        
        /// constructor
        closed_interval( const T& min, const T& max ) : interval_( min, max ) { if( math::less( max, min ) ) { COMMA_THROW( comma::exception, "invalid closed interval" ); } }
        
        /// return value
        const std::pair< T, T >& operator()() const { return interval_; }
        
        /// return left boundary (convenience method)
        T min() const { return interval_.first; }
        
        /// return right boundary (convenience method)
        T max() const { return interval_.second; }
                
        /// return interval size (convenience method)
        T size() const { return interval_.second - interval_.first; }
        
        /// return true, if variable belongs to the interval
        bool contains( const T& t ) const { return !math::less( t, interval_.first ) && !math::less( interval_.second, t ); }
        
        /// return true, if variable belongs to the interval
        bool contains( const closed_interval& rhs ) const { return !( math::less( rhs.first, interval_.first ) || math::less( interval_.second, rhs.second ) ); }
        
        /// compute the hull of the interval and [x]
        closed_interval< T > hull( const T& x ) { return closed_interval( std::min( interval_.first, x ), std::max( interval_.second, x ) ); }

        /// compute the hull of 2 intervals
        closed_interval< T > hull( const closed_interval& rhs ) { return closed_interval( std::min( interval_.first, rhs.min() ), std::max( interval_.second, rhs.max() ) ); }
        
        /// operators
        /// @todo more operators
        bool operator==( const closed_interval& rhs ) const { return math::equal( interval_.first, rhs().first ) && math::equal( interval_.second, rhs().second ); }
        bool operator!=( const closed_interval& rhs ) const { return !operator==( rhs ); }
        
    private:
        std::pair< T, T > interval_;
};

/// project a value into the interval;
/// e.g. 200 degrees projected into [-180, 180) should be -160
template < typename T >
inline T mod( T t, const interval< T >& r );

/// print
template < typename T >
inline std::ostream& operator<<( std::ostream& os, const interval< T >& t ) { os << "[" << t().first << "," << t().second << ")"; return os; }

/// print
template < typename T >
inline std::ostream& operator<<( std::ostream& os, const closed_interval< T >& t ) { os << "[" << t().first << "," << t().second << "]"; return os; }

namespace impl {

inline float mod( float lhs, float rhs ) { return ::fmodf( lhs, rhs ); }
inline double mod( double lhs, double rhs ) { return ::fmod( lhs, rhs ); }
inline long double mod( long double lhs, long double rhs ) { return ::fmodl( lhs, rhs ); }
template < typename T > inline T mod( T lhs, T rhs ) { return lhs % rhs; }
    
} // namespace impl {

template < typename T >
inline T mod( T t, const interval< T >& r )
{
    T size = r.size();
    if( math::less( t, r().first ) )
    {
        return r().second - impl::mod( r().second - t, size );
    }
    return r().first + impl::mod( t - r().first, size );
}

} } // namespace comma { namespace math {

#endif // COMMA_MATH_INTERVAL_H_
