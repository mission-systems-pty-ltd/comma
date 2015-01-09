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
//    This product includes software developed by the University of Sydney.
// 4. Neither the name of the University of Sydney nor the
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

#ifndef COMMA_CSV_IMPL_STATICCAST_HEADER_GUARD_
#define COMMA_CSV_IMPL_STATICCAST_HEADER_GUARD_

#include <string.h>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace comma { namespace csv { namespace impl {

template < typename T > struct static_cast_impl
{
    static const T value( const std::string& s ) { COMMA_THROW( comma::exception, "cannot cast string " << s << " to given type" ); }
    static const T value( const boost::posix_time::ptime& s ) { COMMA_THROW( comma::exception, "cannot cast time " << boost::posix_time::to_iso_string( s ) << " to given type" ); }
    static const T& value( const T& t ) { return t; }
    template < typename S > static T value( const S& s ) { return static_cast< T >( s ); }
};

template <> struct static_cast_impl< std::string >
{
    static const std::string& value( const std::string& t ) { return t; }
    template < typename S > static std::string value( const S& s ) { COMMA_THROW( comma::exception, "cannot cast " << s << " to string" ); }
};

template <> struct static_cast_impl< boost::posix_time::ptime >
{
    static const boost::posix_time::ptime& value( const boost::posix_time::ptime& t ) { return t; }
    template < typename S > static boost::posix_time::ptime value( const S& s ) { COMMA_THROW( comma::exception, "cannot cast " << s << " to time" ); }
};

} } } // namespace comma { namespace csv { namespace impl {

#endif // #ifndef COMMA_CSV_IMPL_STATICCAST_HEADER_GUARD_
