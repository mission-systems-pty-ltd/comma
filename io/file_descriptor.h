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

#ifndef COMMA_IO_FILE_DESCRIPTOR_HEADER
#define COMMA_IO_FILE_DESCRIPTOR_HEADER

#if defined(WIN32)
#if defined(WINCE)
#include <Winsock.h>
#else
#include <Winsock2.h>
#endif
#endif

namespace comma { namespace io {

#ifdef WIN32
    typedef ::SOCKET file_descriptor;
    static const ::SOCKET invalid_file_descriptor = INVALID_SOCKET;
    static const ::SOCKET stdin_fd = INVALID_SOCKET;
    static const ::SOCKET stdout_fd = INVALID_SOCKET;
#else
    typedef int file_descriptor; // according to POSIX standard
    static const file_descriptor invalid_file_descriptor = -1;
    static const file_descriptor  stdin_fd = 0;
    static const file_descriptor  stdout_fd = 1;
#endif    

class has_file_descriptor
{
    public:
        /// constructor
        has_file_descriptor( file_descriptor fd );

        /// return true, if file descriptor is valid
        bool valid() const;

        /// return file descriptor, throw, if invalid
        file_descriptor descriptor() const;

    protected:
        /// invalidate (i.e. when file gets closed)
        void invalidate();

    private:
        file_descriptor fd_;
};

} } // namespace comma { namespace io {

#endif // COMMA_IO_FILE_DESCRIPTOR_HEADER
