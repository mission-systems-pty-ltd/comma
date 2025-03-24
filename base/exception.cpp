// Copyright (c) 2011 The University of Sydney

#include "exception.h"

namespace comma {

exception::exception( const char *message, const char *filename, unsigned long line_number, const char *function_name, bool brief )
    : std::runtime_error( message )
    , _message( message )
    , _filename( filename )
    , _line( line_number )
    , _function( function_name )
{
    _formatted_string( brief );
}

exception::exception( const std::string& message, const char *filename, unsigned long line_number, const char *function_name, bool brief )
    : std::runtime_error( message.c_str() )
    , _message( message )
    , _filename( filename )
    , _line( line_number )
    , _function( function_name )
{
    _formatted_string( brief );
}

const char* exception::what() const throw()
{
    const char* string = "exception::what() _formatted_message.c_str() threw exception";
    try { string = _formatted_message.c_str(); } catch( ... ) {}
    return string;
}

void exception::_formatted_string( bool brief )
{
    std::ostringstream oss;
    oss << error() << std::endl;
    if( !brief )
    {
        oss << "============================================" << std::endl
            << "file: "     << _filename << std::endl
            << "line: "     << _line << std::endl
            << "function: " << _function << std::endl
            << "============================================" << std::endl;
    }
    _formatted_message = oss.str();
}

}  // namespace comma
