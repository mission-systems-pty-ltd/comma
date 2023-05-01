// Copyright (c) 2011 The University of Sydney

/// @author Matthew Herrmann 2007
/// @author Vsevolod Vlaskine 2010-2011

#pragma once

#include <cstring>
#include <type_traits>

namespace comma { namespace packed {

/// packed structure
template < class Derived, size_t S >
class packed_struct
{
    public:
        enum { size = S };

        packed_struct() throw() { static_assert( sizeof( Derived ) == size, "expected derived of provided size" ); }

        const char* data() const throw() { return reinterpret_cast< const char* >( this ); }

        char* data() throw() { return reinterpret_cast< char* >( this ); }

        bool operator==( const packed_struct& rhs ) const { return ::memcmp( this, &rhs, size ) == 0; }

        bool operator!=( const packed_struct& rhs ) const { return !operator==( rhs ); }
};

} } // namespace comma { namespace packed {
