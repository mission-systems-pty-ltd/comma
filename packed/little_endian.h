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
typedef detail::endian< detail::little, 8, false > uint64;
typedef detail::endian< detail::little, 8, true > int64;
typedef detail::endian< detail::little, 4, true, true > float32;
typedef detail::endian< detail::little, 8, true, true > float64;

} // namespace little_endian {

} } // namespace comma { namespace packed {
