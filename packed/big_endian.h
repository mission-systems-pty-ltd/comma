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
typedef detail::endian< detail::big, 8, false > uint64;
typedef detail::endian< detail::big, 8, true > int64;
typedef detail::endian< detail::big, 4, true, true > float32;
typedef detail::endian< detail::big, 8, true, true > float64;

} // namespace big_endian {

} } // namespace comma { namespace packed {
