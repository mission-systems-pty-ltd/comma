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


/// @author vsevolod vlaskine

#ifndef COMMA_BASE_LAST_ERROR_HEADER
#define COMMA_BASE_LAST_ERROR_HEADER

#include <string>
#include "exception.h"

namespace comma {

/// get last error in OS-independent manner
///
/// @todo: add more exception types
/// @todo: do a trick to pass source file name and line number
struct last_error
{
    /// return last error numeric value
    static int value();

    /// return last error as string
    static std::string to_string();

    /// throw last error as a typed exception (use COMMA_THROW_LASTERROR)
    /// @note return bool, because otherwise you may need an extra return
    ///       in the function using to_exception() to get rid of a
    ///       compiler warning
    static void to_exception( const std::string& msg );

    /// generic last error exception
    struct exception;

    /// interrupted system call exception
    struct interrupted_system_call_exception;
};

struct last_error::exception : public comma::exception
{
    exception( const char*, const char *filename, unsigned long line_number, const char *function_name );
    exception( const std::string&, const char *filename, unsigned long line_number, const char *function_name );
};

struct last_error::interrupted_system_call_exception : public last_error::exception
{
    interrupted_system_call_exception( const char*, const char *filename, unsigned long line_number, const char *function_name );
    interrupted_system_call_exception( const std::string&, const char *filename, unsigned long line_number, const char *function_name );
};

} // namespace comma {

#endif // #ifndef COMMA_BASE_LAST_ERROR_HEADER
