// Copyright (c) 2011 The University of Sydney

/// @author vsevolod vlaskine

#if defined(WIN32)
#include "Windows.h"
#include <sstream>
#else
#include "errno.h"
#include "string.h"
#endif

#include <boost/lexical_cast.hpp>
#include "exception.h"
#include "last_error.h"

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

last_error::exception::exception( const char* msg, const char *filename, unsigned long line_number, const char *function_name, bool brief )
    : comma::exception( std::string( msg ) + ": errno " + boost::lexical_cast< std::string >( last_error::value() ) + " - " + last_error::to_string(), filename, line_number, function_name, brief )
{
    value = last_error::value();
}

last_error::exception::exception( const std::string& msg, const char *filename, unsigned long line_number, const char *function_name, bool brief )
    : comma::exception( msg + ": errno " + boost::lexical_cast< std::string >( last_error::value() ) + " - " + last_error::to_string(), filename, line_number, function_name, brief )
{
    value = last_error::value();
}

last_error::interrupted_system_call_exception::interrupted_system_call_exception( const char* msg, const char *filename, unsigned long line_number, const char *function_name, bool brief )
    : last_error::exception( msg, filename, line_number, function_name, brief )
{
}

last_error::interrupted_system_call_exception::interrupted_system_call_exception( const std::string& msg, const char *filename, unsigned long line_number, const char *function_name, bool brief )
    : last_error::exception( msg, filename, line_number, function_name, brief )
{
}

} // namespace comma {
