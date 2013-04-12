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


/// @author Matthew Herrmann 2007
/// @author Vsevolod Vlaskine 2010-2011

#ifndef COMMA_PACKED_FIELD_H_
#define COMMA_PACKED_FIELD_H_

#include <string.h>
#include <boost/static_assert.hpp>

namespace comma { namespace packed {

/// packed field base class
template < typename Derived, typename T, size_t S >
class field
{
    public:
        enum { size = S };

        BOOST_STATIC_ASSERT( size > 0 );

        typedef T type;

        field()
        {
            BOOST_STATIC_ASSERT( sizeof( field ) == size );
            Derived::pack( storage_, Derived::default_value() );
        }

        field( const type& t )
        {
            BOOST_STATIC_ASSERT( sizeof( field ) == size );
            Derived::pack( storage_, t );
        }

        field( const field& rhs )
        {
            BOOST_STATIC_ASSERT( sizeof( field ) == size );
            operator=( rhs );
        }

        type operator()() const { return Derived::unpack( storage_ ); }

        const char* data() const throw() { return storage_; }

        char* data() throw() { return storage_; }

        void pack( char* storage, const type& t );

        type unpack( const char* storage );

        const Derived& operator=( const Derived& rhs ) { ::memcpy( storage_, rhs.storage_, size ); return reinterpret_cast< const Derived& >( *this ); }

        const Derived& operator=( const type& rhs ) { Derived::pack( storage_, rhs ); return reinterpret_cast< const Derived& >( *this ); }

        bool operator==( const Derived& rhs ) { return ::memcmp( storage_, rhs.storage_, size ) == 0; }

        bool operator==( const type& rhs ) { return Derived::unpack( storage_ ) == rhs; }

        bool operator!=( const Derived& rhs ) { return !operator==( rhs ); }

        bool operator!=( const type& rhs ) { return !operator==( rhs ); }

    private:
        char storage_[ size ];
};

} } // namespace comma { namespace packed {

#endif // COMMA_PACKED_FIELD_H_
