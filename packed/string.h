// Copyright (c) 2011 The University of Sydney

/// @author Matthew Herrmann 2007
/// @author Vsevolod Vlaskine 2010-2011
/// @author Dewey Nguyen 2014

#pragma once

#include <cmath>
#include <iomanip>
#include <string>
#include <type_traits>
#include <boost/lexical_cast.hpp>
#include "../base/exception.h"
#include "../string/string.h"
#include "field.h"

namespace comma { namespace packed {

/// packed fixed-length string
template < std::size_t S, char Padding = ' ' >
class string : public packed::field< string< S, Padding >, std::string, S >
{
    public:
        enum { size = S };

        typedef std::string Type;

        typedef packed::field< string< S, Padding >, std::string, S > base_type;

        string() = default;

        string( const std::string& rhs ) { pack( this->data(), rhs ); }

        string( const char* rhs ) { operator=( rhs ); }

        static const std::string& default_value() { static const std::string s; return s; } // static const std::string s( S, Padding );

        static void pack( char* storage, const std::string& value )
        {
            ::memset( storage, Padding, size );
            if( value.length() > size ) { COMMA_THROW_STREAM( comma::exception, "expected not more than " << size << " bytes, got " << value.length() << " (\"" << value << "\")" ); }
            ::memcpy( storage, &value[0], value.size() );
        }

        static std::string unpack( const char* storage ) { return comma::strip( std::string( storage, size ), Padding ); }

        const string& operator=( const std::string& rhs ) { return base_type::operator=( rhs ); }

        const string& operator=( const char* rhs ) { return base_type::operator=( std::string( rhs, size ) ); }

        template < typename T > T as() const { return boost::lexical_cast< T >( this->operator()() ); }
};

/// packed fixed-length string containing a numeric value, e.g. "1234"
/// @note serialized value will be left-aligned and padded
/// @note serialized value will be trunkated to the field size
template < typename T, std::size_t S, char Padding = ' ' >
class casted : public packed::field< casted< T, S, Padding >, T, S >
{
    public:
        enum { size = S };

        typedef T Type;

        typedef packed::field< casted< T, S, Padding >, T, S > base_type;

        static T default_value() { return 0; }

        static void pack( char* storage, const T& value )
        {
            std::string v = boost::lexical_cast< std::string >( value );
            ::memset( storage, Padding, size );
            ::memcpy( storage, &v[0], std::min( std::size_t(size), v.size() ) );
        }

        static T unpack( const char* storage )
        {
            return boost::lexical_cast< T >( comma::strip( std::string( storage, size ), Padding ) );
        }

        const casted& operator=( const T& rhs ) { return base_type::operator=( rhs ); }
};

template < typename T >
inline T hex_to_int( char digit )
{
    if( digit >= '0' && digit <= '9' ) { return digit - '0'; }
    if( digit >= 'a' && digit <= 'f' ) { return digit - 'a' + 10; }
    if( digit >= 'A' && digit <= 'F' ) { return digit - 'A' + 10; }
    COMMA_THROW( comma::exception, "expected hexadecimal digit, got: '" << digit << "'" );
}

template < typename T >
inline T hex_to_int( const char* digits, std::size_t size, char padding = ' ' )
{
    T result = 0;
    const char* end = digits + size;
    for( const char* digit = digits; digit != end && *digit != '\0'; ++digit ) { if( *digit != padding ) { result = result * 16 + hex_to_int< T >( *digit ); } }
    return result;
}

template < typename T >
inline char hex_from_int( T decimal )
{
    if( decimal >= 0 && decimal <= 9 ) { return  '0' + decimal; }
    if( decimal >= 10 && decimal <= 15 ) { return  'a' + decimal - 10; }
    COMMA_THROW( comma::exception, "expected decimal number from 0 to 15, got: '" << decimal << "'" );
}

template < typename T >
inline void hex_from_int( char* storage, std::size_t size, const T& value, char padding = ' ' )
{
    ::memset( storage, padding, size );
    T v = value;
    for( std::size_t i = 1; i <= size; ++i ) { storage[size-i] = hex_from_int< T >( v % 16 ); v /= 16; if( v == 0 ) break; }
    if( v != 0 ) { COMMA_THROW( comma::exception, "decimal number " << value << " cannot be represented with " << size << " hexadecimal character(s)" ); }
}

template < typename T, std::size_t S, char Padding = ' ' >
class ascii_hex : public packed::field< ascii_hex< T, S, Padding >, T, S >
{
public:
    static_assert( boost::is_unsigned< T >::value, "expected unsigned type" );
    enum { size = S };
    
    typedef T Type;
    
    typedef packed::field< casted< T, S, Padding >, T, S > base_type;
    
    static T default_value() { return 0; }
    
    static void pack( char* storage, const T& value )
    {
        return hex_from_int< T >( storage, size, value, Padding );
    }
    
    static T unpack( const char* storage )
    {
        return hex_to_int< T >( storage, size, Padding );
    }
    
    const ascii_hex& operator=( const T& rhs ) { return base_type::operator=( rhs ); }
};

} } // namespace comma { namespace packed {
