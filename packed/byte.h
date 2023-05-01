// Copyright (c) 2011 The University of Sydney

/// @author Vsevolod Vlaskine 2010-2011

#pragma once

#if defined(__linux__) || defined(__APPLE__) || defined(__QNXNTO__)
#include <arpa/inet.h>
#elif defined(WIN32)
#if defined(WINCE)
#include <Winsock.h>
#else
#include <Winsock2.h>
#endif
#endif

#include <type_traits>
#include "field.h"

namespace comma { namespace packed {

/// packed byte
struct byte : public packed::field< byte, unsigned char, sizeof( unsigned char ) >
{
    enum { size = sizeof( unsigned char ) };

    static_assert( size == 1, "expected size 1" );

    typedef unsigned char type;

    typedef packed::field< byte, unsigned char, sizeof( unsigned char ) > base_type;

    static type default_value() { return 0; }

    static void pack( char* storage, type value ) { *storage = static_cast< char >( value ); }

    static type unpack( const char* storage ) { return static_cast< unsigned char >( *storage ); }

    const byte& operator=( const byte& rhs ) { return base_type::operator=( rhs ); }

    const byte& operator=( type rhs ) { return base_type::operator=( rhs ); }
};

/// packed fixed-value byte
template < unsigned char C >
struct const_byte : public packed::field< const_byte< C >, unsigned char, sizeof( unsigned char ) >
{
    enum { size = sizeof( unsigned char ) };

    static_assert( size == 1, "expected size 1" );

    typedef unsigned char type;

    typedef packed::field< const_byte, unsigned char, sizeof( unsigned char ) > base_type;

    const_byte() { base_type::operator=( C ); }

    static type default_value() { return C; }

    static void pack( char* storage, type value ) { *storage = static_cast< char >( value ); }

    static type unpack( const char* storage ) { return static_cast< unsigned char >( *storage ); }

    //const const_byte& operator=( const const_byte& rhs ) { return base_type::operator=( rhs ); }

    //const const_byte& operator=( type rhs ) { return base_type::operator=( rhs ); }
};

typedef byte uint8;

} } // namespace comma { namespace packed {
