// Copyright (c) 2011 The University of Sydney

/// @author Matthew Herrmann 2007
/// @author Vsevolod Vlaskine 2010-2011

#pragma once

#include "detail/endian.h"

namespace comma { namespace packed {

typedef detail::endian< detail::big, 2, false > big_endian_uint16;
typedef detail::endian< detail::big, 2, true > big_endian_int16;
typedef big_endian_uint16 net_uint16;
typedef big_endian_int16 net_int16;
typedef detail::endian< detail::big, 3, false > big_endian_uint24;
typedef detail::endian< detail::big, 3, true > big_endian_int24;
typedef big_endian_uint24 net_uint24;
typedef big_endian_int24 net_int24;
typedef detail::endian< detail::big, 4, false > big_endian_uint32;
typedef detail::endian< detail::big, 4, true > big_endian_int32;
typedef big_endian_uint32 net_uint32;
typedef big_endian_int32 net_int32;
typedef detail::endian< detail::big, 6, false > big_endian_uint48;
typedef detail::endian< detail::big, 6, true > big_endian_int48;
typedef big_endian_uint48 net_uint48;
typedef big_endian_int48 net_int48;
typedef detail::endian< detail::big, 4, true, true > big_endian_float32;
typedef detail::endian< detail::big, 8, true, true > big_endian_float64;
typedef big_endian_float32 big_endian_float;
typedef big_endian_float64 big_endian_double;
typedef big_endian_float32 net_float32;
typedef big_endian_float64 net_float64;
typedef net_float32 net_float;
typedef net_float64 net_double;

// all types above deprecated; use namespacing below
namespace big_endian { // i love namespacing

typedef detail::endian< detail::big, 2, false > uint16;
typedef detail::endian< detail::big, 2, true > int16;
typedef detail::endian< detail::big, 3, false > uint24;
typedef detail::endian< detail::big, 3, true > int24;
typedef detail::endian< detail::big, 4, false > uint32;
typedef detail::endian< detail::big, 4, true > int32;
typedef detail::endian< detail::big, 6, false > uint48; // go figure... there are actual people in the world using it in their protocol packets...
typedef detail::endian< detail::big, 6, true > int48;
typedef detail::endian< detail::big, 8, false > uint64;
typedef detail::endian< detail::big, 8, true > int64;
typedef detail::endian< detail::big, 4, true, true > float32;
typedef detail::endian< detail::big, 8, true, true > float64;

} // namespace big_endian {

} } // namespace comma { namespace packed {
