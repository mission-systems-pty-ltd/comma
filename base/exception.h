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
//    This product includes software developed by the University of Sydney.
// 4. Neither the name of the University of Sydney nor the
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

#ifndef COMMA_BASE_EXCEPTION_H
#define COMMA_BASE_EXCEPTION_H

#include <stdexcept>
#include <sstream>
#include <string>

namespace comma {

#ifndef STRING_HELPER
#define STRING_HELPER(exp) #exp
#endif //STRING_HELPER

#ifndef STRINGIZE
#define STRINGIZE(exp) STRING_HELPER(exp)
#endif //STRINGIZE

#ifndef COMMA_THROW

#if defined( WIN32 )
    #define COMMA_THROW_IMPL_( exception, message )      \
    throw exception( message, __FILE__, __LINE__, __FUNCSIG__ );
#elif defined( __GNUC__ )
    #define COMMA_THROW_IMPL_( exception, message )      \
    throw exception( message, __FILE__, __LINE__, __PRETTY_FUNCTION__ );
#else
    #define COMMA_THROW_IMPL_( exception, message )      \
    throw exception( message, __FILE__, __LINE__, __FUNCTION__ );
#endif

#define COMMA_THROW( exception, strmessage ) { std::ostringstream CommaThrowStr##__LINE__; CommaThrowStr##__LINE__ << strmessage;  COMMA_THROW_IMPL_( exception, CommaThrowStr##__LINE__.str() ); }

#define COMMA_THROW_STREAM( exception, strmessage ) COMMA_THROW( exception, strmessage )

#endif // COMMA_THROW

#ifndef COMMA_RETHROW

#define COMMA_RETHROW() throw;

#endif // COMMA_RETHROW

class exception : public std::runtime_error
{
    public:

        /// constructor
        exception( const char *message, const char *filename, unsigned long line_number, const char *function_name );

        /// constructor
        exception( const std::string& message, const char *filename, unsigned long line_number, const char *function_name );

        /// destructor
        virtual ~exception() throw() {}

        /// e.what is the complete formatted info
        const char*     what(void) const throw();

        /// just the error message
        const char*     error() const;

        /// filename
        const char*     file() const;

        /// line number
        unsigned long   line() const;

        /// function name
        const char*     function() const;

    protected:

        virtual void    formatted_string_();

        std::string     m_message;
        std::string     m_filename;
        unsigned long   m_line_number;
        std::string     m_function_name;
        std::string     m_formatted_message;
};

inline exception::exception( const char *message, const char *filename, unsigned long line_number, const char *function_name ) :
    std::runtime_error( message ),
    m_message( message ),
    m_filename( filename ),
    m_line_number( line_number ),
    m_function_name( function_name )
{
    formatted_string_();
}

inline exception::exception( const std::string& message, const char *filename, unsigned long line_number, const char *function_name ) :
    std::runtime_error( message.c_str() ),
    m_message( message ),
    m_filename( filename ),
    m_line_number( line_number ),
    m_function_name( function_name )
{
    formatted_string_();
}

inline const char* exception::what(void) const throw()
{
    const char * string = "exception::what() m_formatted_message.c_str() threw exception";
    try
    {
      string = m_formatted_message.c_str();
    }
    catch( ... )
    {}
    return string;
}

inline const char* exception::error() const
{
    return m_message.c_str();
}


inline const char* exception::file() const
{
    return m_filename.c_str();
}

inline unsigned long exception::line() const
{
    return m_line_number;
}

inline const char* exception::function() const
{
    return m_function_name.c_str();
}

inline void exception::formatted_string_()
{
    std::ostringstream oss;
    oss << error() << std::endl
        << "============================================" << std::endl
        << "file: "     << m_filename << std::endl
        << "line: "     << m_line_number << std::endl
        << "function: " << m_function_name << std::endl
        << "============================================" << std::endl;
    m_formatted_message = oss.str();
}

}  // namespace comma

#endif //COMMA_BASE_EXCEPTION_H

