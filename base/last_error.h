// Copyright (c) 2011 The University of Sydney

/// @author vsevolod vlaskine

#pragma once

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
    exception( const char*, const char *filename, unsigned long line_number, const char *function_name, bool brief = false );
    exception( const std::string&, const char *filename, unsigned long line_number, const char *function_name, bool brief = false );

    int value;
};

struct last_error::interrupted_system_call_exception : public last_error::exception
{
    interrupted_system_call_exception( const char*, const char *filename, unsigned long line_number, const char *function_name, bool brief = false );
    interrupted_system_call_exception( const std::string&, const char *filename, unsigned long line_number, const char *function_name, bool brief = false );
};

} // namespace comma {
