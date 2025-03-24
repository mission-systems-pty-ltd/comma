// Copyright (c) 2011 The University of Sydney

/// @author vsevolod vlaskine

#pragma once

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
    #define COMMA_THROW_IMPL_( exception, message, brief )      \
    throw exception( message, __FILE__, __LINE__, __FUNCSIG__, brief );
#elif defined( __GNUC__ )
    #define COMMA_THROW_IMPL_( exception, message, brief )      \
    throw exception( message, __FILE__, __LINE__, __PRETTY_FUNCTION__, brief );
#else
    #define COMMA_THROW_IMPL_( exception, message, brief )      \
    throw exception( message, __FILE__, __LINE__, __FUNCTION__, brief );
#endif

#define COMMA_THROW( exception, strmessage ) { std::ostringstream CommaThrowStr##__LINE__; CommaThrowStr##__LINE__ << strmessage;  COMMA_THROW_IMPL_( exception, CommaThrowStr##__LINE__.str(), false ); }

#define COMMA_THROW_BRIEF( exception, strmessage ) { std::ostringstream CommaThrowStr##__LINE__; CommaThrowStr##__LINE__ << strmessage;  COMMA_THROW_IMPL_( exception, CommaThrowStr##__LINE__.str(), true ); }

#define COMMA_THROW_STREAM( exception, strmessage ) COMMA_THROW( exception, strmessage )

#define COMMA_THROW_STREAM_BRIEF( exception, strmessage ) COMMA_THROW_BRIEF( exception, strmessage )

#endif // COMMA_THROW

#ifndef COMMA_RETHROW

#define COMMA_RETHROW() throw;

#endif // COMMA_RETHROW

#define COMMA_ASSERT( condition, strmessage ) { if( !( condition ) ) { COMMA_THROW( comma::exception, "condition: '" << #condition << "' is false; " << strmessage ); } }

#define COMMA_ASSERT_BRIEF( condition, strmessage ) { if( !( condition ) ) { COMMA_THROW_BRIEF( comma::exception, strmessage ); } }

#define COMMA_THROW_IF( condition, strmessage ) { if( condition ) { COMMA_THROW( comma::exception, "throw because condition: '" << #condition << "' is true; " << strmessage ); } }

#define COMMA_THROW_BRIEF_IF( condition, strmessage ) { if( condition ) { COMMA_THROW_BRIEF( comma::exception, strmessage ); } }

class exception : public std::runtime_error
{
    public:

        /// constructor
        exception( const char *message, const char *filename, unsigned long line_number, const char *function_name, bool brief = false );

        /// constructor
        exception( const std::string& message, const char *filename, unsigned long line_number, const char *function_name, bool brief = false );

        /// destructor
        virtual ~exception() throw() {}

        /// e.what is the complete formatted info
        const char*     what() const throw();

        /// just the error message
        const char*     error() const { return &_message[0]; }

        /// filename
        const char*     file() const { return &_filename[0]; }

        /// line number
        unsigned long   line() const { return _line; }

        /// function name
        const char*     function() const { return &_function[0]; }

    protected:

        virtual void    _formatted_string( bool brief );

        std::string     _message;
        std::string     _filename;
        unsigned long   _line{0};
        std::string     _function;
        std::string     _formatted_message;
};

}  // namespace comma
