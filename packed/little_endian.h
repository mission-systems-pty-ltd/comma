// Copyright (c) 2011 The University of Sydney

/// @author Matthew Herrmann 2007
/// @author vsevolod vlaskine

#pragma once

#include "detail/endian.h"

namespace comma { namespace packed {

typedef detail::endian< detail::little, 2, true > little_endian16;
typedef detail::endian< detail::little, 2, false > little_endian_uint16;
typedef little_endian16 int16;
typedef little_endian_uint16 uint16;
typedef detail::endian< detail::little, 3, true > little_endian24;
typedef detail::endian< detail::little, 3, false > little_endian_uint24;
typedef little_endian24 int24;
typedef little_endian_uint24 uint24;
typedef detail::endian< detail::little, 4, true > little_endian32;
typedef detail::endian< detail::little, 4, false > little_endian_uint32;
typedef little_endian32 int32;
typedef little_endian_uint32 uint32;
typedef detail::endian< detail::little, 6, true > little_endian48;
typedef detail::endian< detail::little, 6, false > little_endian_uint48;
typedef little_endian48 int48;
typedef little_endian_uint48 uint48;
typedef detail::endian< detail::little, 8, true > little_endian64;
typedef detail::endian< detail::little, 8, false > little_endian_uint64;
typedef little_endian64 int64;
typedef little_endian_uint64 uint64;
typedef detail::endian< detail::little, 4, true, true > little_endian_float32;
typedef detail::endian< detail::little, 8, true, true > little_endian_float64;
typedef little_endian_float32 float32;
typedef little_endian_float64 float64;

// all types above deprecated; use namespacing below
namespace little_endian { // i love namespacing

typedef detail::endian< detail::little, 2, false > uint16;
typedef detail::endian< detail::little, 2, true > int16;
typedef detail::endian< detail::little, 3, false > uint24;
typedef detail::endian< detail::little, 3, true > int24;
typedef detail::endian< detail::little, 4, false > uint32;
typedef detail::endian< detail::little, 4, true > int32;
typedef detail::endian< detail::little, 6, false > uint48;
typedef detail::endian< detail::little, 6, true > int48;
typedef detail::endian< detail::little, 8, false > uint64;
typedef detail::endian< detail::little, 8, true > int64;
typedef detail::endian< detail::little, 4, true, true > float32;
typedef detail::endian< detail::little, 8, true, true > float64;

} // namespace little_endian {

} } // namespace comma { namespace packed {
