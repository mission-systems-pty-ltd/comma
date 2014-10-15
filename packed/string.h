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


/// @author Matthew Herrmann 2007
/// @author Vsevolod Vlaskine 2010-2011
/// @author Dewey Nguyen 2014-2014

#ifndef COMMA_PACKED_STRING_H_
#define COMMA_PACKED_STRING_H_

#include <cmath>
#include <string>
#include <boost/lexical_cast.hpp>
#include <comma/base/exception.h>
#include <comma/packed/field.h>
#include <comma/string/string.h>
#include <iomanip>

namespace comma { namespace packed {

/// packed fixed-length string
template < std::size_t S, char Padding = ' ' >
class string : public packed::field< string< S, Padding >, std::string, S >
{
    public:
        enum { size = S };

        typedef std::string Type;

        typedef packed::field< string< S, Padding >, std::string, S > base_type;

        static const std::string& default_value()
        {
            static const std::string s;// static const std::string s( S, Padding );
            return s;
        }

        static void pack( char* storage, const std::string& value )
        {
            ::memset( storage, Padding, size );
            if( value.length() > size ) { COMMA_THROW_STREAM( comma::exception, "expected not more than " << size << " bytes, got " << value.length() << " (\"" << value << "\")" ); }
            ::memcpy( storage, &value[0], value.size() );
        }

        static std::string unpack( const char* storage )
        {
            return comma::strip( std::string( storage, size ), Padding );
        }

        const string& operator=( const std::string& rhs ) { return base_type::operator=( rhs ); }

        const string& operator=( const char* rhs ) { return base_type::operator=( std::string( rhs, size ) ); }

        /// a convenience method, if string represents numeric values
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

inline unsigned int hex_to_int( char digit )
{
    if( digit >= '0' && digit <= '9' ) { return digit - '0'; }
    if( digit >= 'a' && digit <= 'f' ) { return digit - 'a' + 10; }
    if( digit >= 'A' && digit <= 'F' ) { return digit - 'A' + 10; }
    COMMA_THROW( comma::exception, "expected hexadecimal digit, got: '" << digit << "'" );
}

inline unsigned int hex_to_int( const char* digits, unsigned int size, char padding = ' ' )
{
    unsigned int result = 0;
    const char* end = digits + size;
    for( const char* digit = digits; digit != end && *digit != '\0'; ++digit ) { if( *digit != padding ) { result = result * 16 + hex_to_int( *digit ); } }
    return result;
}

inline char hex_from_int( unsigned int digit )
{
    if( digit >= 0 && digit <= 9 ) { return  '0' + digit; }
    if( digit >= 10 && digit <= 15 ) { return  'a' + digit - 10; }
}

template < typename T >
inline void hex_from_int( char* storage, unsigned int size, const T& value, char padding = ' ' )
{
    unsigned int i = 1;
    for( T v = value; i <= size; ) { storage[size-i] = hex_from_int( v % 16 ); ++i; v /= 16; if( v == 0 ) break; }
    for( ; i <= size; ++i ) { storage[size-i] = padding; }
}

template < typename T, std::size_t S, char Padding = ' ' >
class ascii_hex : public packed::field< ascii_hex< T, S, Padding >, T, S >
{
public:
    BOOST_STATIC_ASSERT( boost::is_unsigned< T >::value );
    enum { size = S };
    
    typedef T Type;
    
    typedef packed::field< casted< T, S, Padding >, T, S > base_type;
    
    static T default_value() { return 0; }
    
    static void pack( char* storage, const T& value )
    {
        return hex_from_int( storage, size, value, Padding );
//         std::ostringstream ss;
//         ss << std::hex << std::setfill( Padding ) << std::setw( size ) << value;
//         ::memcpy( storage, &(ss.str()[0]), size );
    }
    
    static T unpack( const char* storage )
    {
        return hex_to_int( storage, size, Padding );
//         std::istringstream iss( comma::strip( std::string( storage, size ), Padding ) );
//         T value;
//         iss >> std::hex >> value;
//         return value;
    }
    
    const ascii_hex& operator=( const T& rhs ) { return base_type::operator=( rhs ); }
};

} } // namespace comma { namespace packed {

#endif // #ifndef COMMA_PACKED_STRING_H_
