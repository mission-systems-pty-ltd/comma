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


/// @author vsevolod vlaskine

#if defined(WIN32)
#include "Windows.h"
#include <sstream>
#else
#include "errno.h"
#include "string.h"
#endif

#include <comma/base/exception.h>
#include "comma/base/last_error.h"

namespace comma {

int last_error::value()
{
    #if defined(WIN32)
    return GetLastError();
    #else
    return errno;
    #endif
}

std::string last_error::to_string()
{
    #if defined(WIN32)
    std::ostringstream oss;
    oss << "Windows error #" << value();
    return oss.str();
    #else
    return ::strerror( errno );
    #endif
}

void last_error::to_exception( const std::string& msg )
{
    #ifdef WIN32
    switch( value() )
    {
        // TODO: add more exceptions
        case 0: break;
        case WSAEINTR: COMMA_THROW( last_error::interrupted_system_call_exception, msg );
        default: COMMA_THROW( last_error::exception, msg );
    }
    #else
    switch( value() )
    {
        // TODO: add more exceptions
        case 0: break;
        case EINTR: COMMA_THROW( last_error::interrupted_system_call_exception, msg );
        default: COMMA_THROW( last_error::exception, msg );
    };
    #endif
}

last_error::exception::exception( const char* msg, const char *filename, unsigned long line_number, const char *function_name )
    : comma::exception( std::string( msg ) + ": " + last_error::to_string(), filename, line_number, function_name )
{
}

last_error::exception::exception( const std::string& msg, const char *filename, unsigned long line_number, const char *function_name )
    : comma::exception( msg + ": " + last_error::to_string(), filename, line_number, function_name )
{
}

last_error::interrupted_system_call_exception::interrupted_system_call_exception( const char* msg, const char *filename, unsigned long line_number, const char *function_name )
    : last_error::exception( msg, filename, line_number, function_name )
{
}

last_error::interrupted_system_call_exception::interrupted_system_call_exception( const std::string& msg, const char *filename, unsigned long line_number, const char *function_name )
    : last_error::exception( msg, filename, line_number, function_name )
{
}

} // namespace comma {
