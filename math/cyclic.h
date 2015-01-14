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

#ifndef COMMA_MATH_CYCLIC_H_
#define COMMA_MATH_CYCLIC_H_

#include <iostream>
#include <comma/base/exception.h>
#include <comma/math/compare.h>
#include <comma/math/interval.h>

namespace comma { namespace math {

/// cyclic value for discrete types
template < typename T >
class cyclic
{
    public:
        /// constructor
        cyclic( const math::interval< T >& r ) : interval_( r ), value_( r().first ) {}
    
        /// constructor
        cyclic( const T& first, const T& second ) : interval_( first, second ), value_( first ) {}
    
        /// constructor
        cyclic( const math::interval< T >& r, const T& t ) : interval_( r ), value_( math::mod( t, interval_ ) ) {}
    
        /// copy constructor
        cyclic( const cyclic& rhs ) : interval_( rhs.interval() ) { operator=( rhs ); }
    
        /// return value
        const T& operator()() const { return value_; }
    
        /// return interval
        const math::interval< T >& interval() const { return interval_; }
    
        /// operators
        const cyclic& operator=( const cyclic& rhs ) { interval_ = rhs.interval_; value_ = rhs.value_; return *this; }
        const cyclic& operator=( const T& t ) { value_ = math::mod( t, interval_ ); return *this; }
        bool operator==( const cyclic& rhs ) const { return equal( interval_, rhs.interval_ ) && equal( value_, rhs.value_ ); }
        bool operator!=( const cyclic& rhs ) const { return !operator==( rhs ); }
        bool between( const cyclic& lower, const cyclic& upper ) const;
        const cyclic& operator++() { operator+=( 1 ); return *this; }
        cyclic operator++( int ) { T v = value_; operator++(); return v; }
        const cyclic& operator--() { operator-=( 1 ); return *this; }
        cyclic operator--( int ) { T v = value_; operator--(); return v; }
        const cyclic& operator+=( const cyclic& rhs ) { compatible( rhs, "+=" ); return operator+=( rhs() - interval_().first ); }
        const cyclic& operator-=( const cyclic& rhs ) { compatible( rhs, "-=" ); return operator-=( rhs() - interval_().first ); }
        const cyclic& operator+=( const T& t )
        {
            value_ = math::mod( value_ + t, interval_ );
            return *this;
        }
        const cyclic& operator-=( const T& t )
        {
            T s = math::mod( t, interval_ );
            value_ = ( math::less( value_, s ) ? interval_().second : interval_().first ) + value_ - s;
            return *this;
        }
        cyclic operator+( const cyclic& rhs ) const { cyclic v( *this ); v += rhs; return v; }
        cyclic operator+( const T& rhs ) const { cyclic v( *this ); v += rhs; return v; }
        cyclic operator-( const cyclic& rhs ) const { cyclic v( *this ); v -= rhs; return v; }
        cyclic operator-( const T& rhs ) const { cyclic v( *this ); v -= rhs; return v; }
    
    private:
        math::interval< T > interval_;
        T value_;
        void compatible( const cyclic& rhs, const char* op )
        {
            if( interval_ != rhs.interval() )
            {
                COMMA_THROW( comma::exception, "operation \"" << op << "\": incompatible cyclic variables" );
            }
        }
};

/// print
template < typename T >
std::ostream& operator<<( std::ostream& os, const cyclic< T >& t ) { os << t() << " in " << t.interval(); return os; }

template < typename T >
bool cyclic< T >::between( const cyclic< T >& lower, const cyclic< T >& upper ) const
{
    if( less( upper.value_, lower.value_ ) )
    {
        return !less( value_, lower.value_ ) || less( value_, upper.value_ );
    }
    else
    {
        return !less( value_, lower.value_ ) && less( value_, upper.value_ );
    }
}

} } // namespace comma { namespace math {

#endif // COMMA_MATH_CYCLIC_H_
