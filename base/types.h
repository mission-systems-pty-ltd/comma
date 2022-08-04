// Copyright (c) 2011 The University of Sydney

/// @author vsevolod vlaskine

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

#include <cmath>
#include <limits>
#include <type_traits>

namespace comma {

#if defined(__linux__) || defined(__APPLE__) || defined(__QNXNTO__)

typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

#elif defined(WIN32)

typedef unsigned __int16 uint16;
typedef unsigned __int32 uint32;
typedef unsigned __int64 uint64;

typedef __int16 int16;
typedef __int32 int32;
typedef __int64 int64;

// Windows, you know...
static_assert( sizeof( uint16 ) == 2, "expected uint16 of size 2" );
static_assert( sizeof( uint32 ) == 4, "expected uint32 of size 4" );
static_assert( sizeof( uint64 ) == 8, "expected uint64 of size 8" );
static_assert( sizeof( int16 ) == 2, "expected int16 of size 2" );
static_assert( sizeof( int32 ) == 4, "expected int32 of size 4" );
static_assert( sizeof( int64 ) == 8, "expected int64 of size 8" );

#endif

template < unsigned int Size, bool Signed = true > struct integer {};
template <> struct integer< 1, true > { typedef char type; };
template <> struct integer< 1, false > { typedef unsigned char type; };
template <> struct integer< 2, true > { typedef comma::int16 type; };
template <> struct integer< 2, false > { typedef comma::uint16 type; };
template <> struct integer< 4, true > { typedef comma::int32 type; };
template <> struct integer< 4, false > { typedef comma::uint32 type; };
template <> struct integer< 8, true > { typedef comma::int64 type; };
template <> struct integer< 8, false > { typedef comma::uint64 type; };

} // namespace comma {
