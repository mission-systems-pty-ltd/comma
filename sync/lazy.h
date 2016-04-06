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

#ifndef COMMA_SYNC_LAZY_HEADER_GUARD_
#define COMMA_SYNC_LAZY_HEADER_GUARD_

#include <boost/function.hpp>
#include <boost/optional.hpp>

namespace comma {

/// trivial lazy initialization wrapper
/// \note life span of construction parameters pass by pointer or reference is the same
///       as for boost::function and boost::bind, i.e. the user needs to make sure
///       that construction parameters do not go out of scope at the places of usage
///       of lazy variables
template < typename T >
class lazy
{
    public:
        typedef boost::function< T() > functor_t;
        
        lazy() {}
        
        lazy( const functor_t& make_value ) : make_value_( make_value ) {}
        
        T& get() { make_(); return *value_; }
        
        const T& get() const { make_(); return *value_; }
        
        operator T() { return get(); }
        
        operator T() const { return get(); }
        
        T& operator*() { return get(); }
        
        const T& operator*() const { return get(); }
        
        T* operator->() { return &get(); }
        
        const T* operator->() const { return &get(); }
        
    private:
        boost::optional< T > value_;
        functor_t make_value_;
        void init_( const functor_t& make_value ) { make_value_ = make_value; }
        void make_() const
        { 
            if( value_ ) { return; }
            if( make_value_ ) { const_cast< boost::optional< T >& >( value_ ).reset( make_value_() ); }
            else { const_cast< boost::optional< T >& >( value_ ) = T(); }
        }
};

template < typename T >
template < typename A1, typename A2 >
lazy::lazy( const A1& a1, const A2& a2 );

} // namespace comma {

#endif // #ifndef COMMA_SYNC_LAZY_HEADER_GUARD_
