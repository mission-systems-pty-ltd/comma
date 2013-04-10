// This file is part of comma
//
// Copyright (C) 2011 The University of Sydney
//
// comma is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// comma is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
// for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with comma. If not, see <http://www.gnu.org/licenses/>.

/// @author Matthew Herrmann 2007
/// @author Vsevolod Vlaskine 2010-2011

#ifndef COMMA_PACKED_STRING_H_
#define COMMA_PACKED_STRING_H_

#include <cmath>
#include <string>
#include <boost/lexical_cast.hpp>
#include <comma/base/exception.h>
#include <comma/packed/field.h>
#include <comma/string/string.h>

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
            ::memcpy( storage, &v[0], std::min( size, v.size() ) );
        }

        static T unpack( const char* storage )
        {
            return boost::lexical_cast< T >( comma::strip( std::string( storage, size ), Padding ) );
        }

        const casted& operator=( const T& rhs ) { return base_type::operator=( rhs ); }
};

} } // namespace comma { namespace packed {

#endif // #ifndef COMMA_PACKED_STRING_H_
