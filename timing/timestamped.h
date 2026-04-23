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

#pragma once

#include <chrono>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace comma {

template < typename Time > struct time_traits;

template <> struct time_traits< boost::posix_time::ptime >
{
    static boost::posix_time::ptime system_clock() { return boost::posix_time::microsec_clock::universal_time(); }
};

template <> struct time_traits< std::chrono::system_clock::time_point >
{
    static std::chrono::system_clock::time_point system_clock() { return std::chrono::system_clock::now(); }
};

template < typename T, typename Time = boost::posix_time::ptime >
struct timestamped
{
    typedef Time timestamp_t;

    timestamp_t t;

    T data;

    timestamped() {}
    timestamped( const T& data ) : t( time_traits< Time >::system_clock() ), data( data ) {}
    timestamped( T&& data ) : t( time_traits< Time >::system_clock() ), data( data ) {}
    timestamped( timestamp_t t, const T& data ) : t( t ), data( data ) {}
    timestamped( timestamp_t t, T&& data ) : t( t ), data( data ) {}
    template < typename S > bool operator<( const timestamped< S >& s ) const { return t < s.t; }
    template < typename S > bool operator==( const timestamped< S >& s ) const { return t == s.t; }
    template < typename S > bool operator<=( const timestamped< S >& s ) const { return t <= s.t; }
    template < typename S > bool operator>( const timestamped< S >& s ) const { return t > s.t; }
    template < typename S > bool operator>=( const timestamped< S >& s ) const { return t >= s.t; }
};

template < typename T > inline timestamped< T > make_timestamped( T&& data ) { return timestamped< T >( data ); }

template < typename T > inline timestamped< T > make_timestamped( boost::posix_time::ptime t, T&& data ) { return timestamped< T >( t, data ); }

template < typename T > inline timestamped< T, std::chrono::system_clock::time_point > make_timestamped( std::chrono::system_clock::time_point t, T&& data ) { return timestamped< T, std::chrono::system_clock::time_point >( t, data ); }

} // namespace comma {
