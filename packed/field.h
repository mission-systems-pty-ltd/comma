// Copyright (c) 2011 The University of Sydney

/// @author Matthew Herrmann 2007
/// @author Vsevolod Vlaskine 2010-2011

#pragma once

#include <string.h>
#include <type_traits> // #include <boost/static_assert.hpp>

namespace comma { namespace packed {

/// packed field base class
template < typename Derived, typename T, size_t S >
class field
{
    public:
        enum { size = S };

        static_assert( size > 0, "expected positive size" );

        typedef T type;

        field()
        {
            static_assert( sizeof( field ) == size, "field size does not match stated size" );
            Derived::pack( storage_, Derived::default_value() );
        }

        field( const type& t )
        {
            static_assert( sizeof( field ) == size, "field size does not match stated size" );
            Derived::pack( storage_, t );
        }

        field( const field& rhs )
        {
            static_assert( sizeof( field ) == size, "field size does not match stated size" );
            operator=( rhs );
        }

        type operator()() const { return Derived::unpack( storage_ ); }

        const char* data() const throw() { return storage_; }

        char* data() throw() { return storage_; }

        void pack( char* storage, const type& t );

        type unpack( const char* storage );

        const Derived& operator=( const field& rhs ) { return operator=( reinterpret_cast< const Derived& >( rhs ) ); }

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
