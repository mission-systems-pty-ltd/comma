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

#ifndef COMMA_CSV_APPLICATIONS_FORMAT_HEADER_GUARD_
#define COMMA_CSV_APPLICATIONS_FORMAT_HEADER_GUARD_

#include <stdlib.h>
#include <iostream>
#include <set>
#include <string>
#include <vector>
#include <boost/optional.hpp>
#include <boost/type_traits.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "../base/exception.h"
#include "../base/types.h"
#include "../string/string.h"
#include "../visiting/apply.h"
#include "../visiting/visit.h"
#include "../visiting/while.h"
#include "../xpath/xpath.h"

namespace comma { namespace csv {

/// forward declaration
namespace impl { class to_format; }

/// csv to/from bin format
/// @todo the interface has got real messy; fully refactor!
class format
{
    public:
        /// types (implement more, as we need them)
        /// note: currently string type is for fixed size string only
        ///       a variable size string is tricky and we may never implement it for csv
        enum types_enum { char_t, int8, uint8, int16, uint16, int32, uint32, int64, uint64, float_t, double_t, time, long_time, fixed_string };

        /// type to enum
        template < typename T > struct type_to_enum {};

        /// format element traits
        template < typename T, types_enum F = type_to_enum< T >::value > struct traits
        {
            static const types_enum type = F;
            static const unsigned int size = sizeof( T );
            static const char* as_string() { return type_to_enum< T >::as_string(); }
            //static T from_bin( const char* buf, std::size_t size = sizeof( T ) ) { T t; ::memcpy( &t, buf, sizeof( T ) ); return t; }
            //static void to_bin( const T& t, char* buf, std::size_t size = sizeof( T ) ) { ::memcpy( buf, &t, sizeof( T ) ); }
            static T from_bin( const char* buf, std::size_t size = sizeof( T ) ) { (void)size; return *reinterpret_cast< const T* >( buf ); }
            static void to_bin( const T& t, char* buf, std::size_t size = sizeof( T ) ) { (void)size; *reinterpret_cast< T* >( buf ) = t; }
        };

        /// struct containing offsets
        struct element
        {
            element() : offset( 0 ), count( 0 ), size( 0 ) {}
            element( std::size_t o, std::size_t c, std::size_t s, types_enum type ) : offset( o ), count( c ), size( s ), type( type ) {}
            std::size_t offset; /// offset of the 1st element
            std::size_t count;  /// number of elements; e.g. as in "3d" size will be count * size, i.e count * sizeof(double)
            std::size_t size;   /// element size; e.g. as in "3d" size will be count * size, i.e count * sizeof(double)
            types_enum type; /// element type
        };

        /// constructor
        format( const std::string& format = "" );
        
        /// return format elements
        const std::vector< element >& elements() const;
        
        /// return offset and size for a given field (thus, count always will be 1)
        element offset( std::size_t i ) const;
        
        /// quick and dirty: return index and count of offset for an element
        std::pair< unsigned int, unsigned int > index( std::size_t i ) const;
        
        /// return binary buffer size
        std::size_t size() const;

        /// return number of fields
        std::size_t count() const;

        /// append string (a convenience method)
        const format& operator+=( const std::string& rhs );

        /// append format (a convenience method)
        const format& operator+=( const format& rhs );
        
        /// return size of binary field
        static std::size_t size_of( types_enum type );
        
        /// take csv string, write to the given stream
        void csv_to_bin( std::ostream& os, const std::string& csv, char delimiter = ',', bool flush = false ) const;
        void csv_to_bin( std::ostream& os, const std::vector< std::string >& csv, bool flush = false ) const;
        std::string csv_to_bin( const std::string& csv, char delimiter = ',' ) const;
        std::string csv_to_bin( const std::vector< std::string >& csv ) const;
        
        /// take binary string, return csv
        std::string bin_to_csv( const char* bin, char delimiter = ',', const boost::optional< unsigned int >& precision = boost::optional< unsigned int >() ) const;
        
        /// take binary string, return csv
        std::string bin_to_csv( const std::string& bin, char delimiter = ',', const boost::optional< unsigned int >& precision = boost::optional< unsigned int >() ) const;

        /// return as string
        const std::string& string() const;

        /// return as a string in fully expanded format, e.g. "2d,3f" -> "d,d,f,f,f"
        std::string expanded_string() const;

        /// return as a string in minimal format, e.g. "d,d,f,f,f" -> "2d,3f"
        std::string collapsed_string() const;
        
        /// return format usage
        static std::string usage();
        
        /// return value for a type
        template < typename T >
        static std::string value( const T& t = T() );

        /// return value for a type
        template < typename T >
        static std::string value( const std::string& fields, bool full_xpath, const T& t = T() );

        /// return format for a type
        static std::string to_format( types_enum type );

        /// return format for a type
        static std::string to_format( types_enum type, unsigned int size );
                
    private:
        std::string string_;
        std::vector< types_enum > types_;
        std::vector< element > elements_;
        std::size_t size_;
        std::size_t count_;
        std::size_t elements_number_; /// total number of elements
        friend class impl::to_format;
        template < typename T > static std::string value_impl( const T& t );
};


namespace impl {

class to_format
{
    public:
        to_format( const std::string& fields = "", bool full_xpath = true )
            : full_xpath_( full_xpath )
        {
            if( fields.empty() ) { return; }
            std::vector< std::string > v = comma::split( fields, ',' );
            for( std::size_t i = 0; i < v.size(); ++i )
            { 
                if( v[i].empty() ) { COMMA_THROW( comma::exception, "expected all fields non-empty, got field " << i << " empty" ); }
                fields_[ v[i] ] = i;
            }
            elements_.resize( fields_.size() );
        }
        
        template < typename K, typename T >
        void apply( const K& name, const boost::optional< T >& value )
        {
            append( name );
            visiting::do_while<    !boost::is_fundamental< T >::value
                                && !boost::is_same< T, std::string >::value
                                && !boost::is_same< T, boost::posix_time::ptime >::value >::visit( name, value ? *value : T(), *this );
            trim( name );
        }
        
        template < typename K, typename T >
        void apply( const K& name, const T& value )
        {
            append( name );
            visiting::do_while<    !boost::is_fundamental< T >::value
                                && !boost::is_same< T, std::string >::value
                                && !boost::is_same< T, boost::posix_time::ptime >::value >::visit( name, value, *this );
            trim( name );
        }
        
        template < typename K, typename T >
        void apply_next( const K& name, const T& value ) { comma::visiting::visit( name, value, *this ); }
        
        template < typename K, typename T >
        void apply_final( const K& key, const T& value )
        {
            (void)key;
            if( fields_.empty() )
            {
                format_ += ( format_.empty() ? "" : "," ) + format::value_impl( value );
            }
            else
            {
                map_t_::const_iterator it = fields_.end();
                if( full_xpath_ ) { for( it = fields_.begin(); it != fields_.end(); ++it ) { if( xpath_ <= comma::xpath( it->first ) ) { break; } } }
                else { it = fields_.find( xpath_.elements.back().to_string() ); }
                if( it != fields_.end() ) { elements_[ it->second ] += ( elements_[ it->second ].empty() ? "" : "," ) + format::value_impl( value ); }
            }
        }
        
        std::string operator()() const { return format_.empty() ? comma::join( elements_, ',' ) : format_; }
        
    private:
        typedef std::map< std::string, unsigned int > map_t_;
        map_t_ fields_;
        bool full_xpath_;
        std::string format_;
        std::vector< std::string > elements_;
        xpath xpath_;
        const xpath& append( std::size_t index ) { xpath_.elements.back().index = index; return xpath_; }
        const xpath& append( const char* name ) { xpath_ /= xpath::element( name ); return xpath_; }
        const xpath& append( const std::string& name ) { xpath_ /= xpath::element( name ); return xpath_; }
        const xpath& trim( std::size_t ) { xpath_.elements.back().index = boost::optional< std::size_t >(); return xpath_; }
        const xpath& trim( const char* ) { xpath_ = xpath_.head(); return xpath_; }
        const xpath& trim( const std::string& ) { xpath_ = xpath_.head(); return xpath_; }
};

} // namespace impl {

template < typename T >
inline std::string format::value( const T& t ) { return value( "", true, t ); }

template < typename T >
inline std::string format::value( const std::string& fields, bool full_xpath, const T& t )
{
    impl::to_format v( fields, full_xpath );
    visiting::apply( v, t );
    return v();
}

template <> inline std::string format::value_impl< bool >( const bool& ) { return "b"; }
template <> inline std::string format::value_impl< char >( const char& ) { return "b"; }
template <> inline std::string format::value_impl< unsigned char >( const unsigned char& ) { return "ub"; }
template <> inline std::string format::value_impl< int16 >( const comma::int16& ) { return "w"; }
template <> inline std::string format::value_impl< comma::uint16 >( const comma::uint16& ) { return "uw"; }
template <> inline std::string format::value_impl< int32 >( const comma::int32& ) { return "i"; }
template <> inline std::string format::value_impl< comma::uint32 >( const comma::uint32& ) { return "ui"; }
template <> inline std::string format::value_impl< int64 >( const comma::int64& ) { return "l"; }
template <> inline std::string format::value_impl< comma::uint64 >( const comma::uint64& ) { return "ul"; }
template <> inline std::string format::value_impl< float >( const float& ) { return "f"; }
template <> inline std::string format::value_impl< double >( const double& ) { return "d"; }
template <> inline std::string format::value_impl< long double >( const long double& ) { return "d"; }
template <> inline std::string format::value_impl< boost::posix_time::ptime >( const boost::posix_time::ptime& ) { return "t"; }
template <> inline std::string format::value_impl< std::string >( const std::string& s )
{ // quick and dirty
    if( s.empty() ) { return "s"; } // variable size string, todo
    std::ostringstream oss; // fixed size string, quick and dirty
    oss << "s[" << s.length() << "]"; //oss << "%" << s.length() << "c";
    return oss.str();
}

template <> struct format::type_to_enum< bool >
{
    static const format::types_enum value = format::int8;
    static const char* as_string() { return "b"; }
};

template <> struct format::type_to_enum< char >
{
    static const format::types_enum value = format::int8;
    static const char* as_string() { return "b"; }
};

template <> struct format::type_to_enum< unsigned char >
{
    static const format::types_enum value = format::uint8;
    static const char* as_string() { return "ub"; }
};

template <> struct format::type_to_enum< comma::int16 >
{
    static const format::types_enum value = format::int16;
    static const char* as_string() { return "w"; }
};

template <> struct format::type_to_enum< comma::uint16 >
{
    static const format::types_enum value = format::uint16;
    static const char* as_string() { return "uw"; }
};

template <> struct format::type_to_enum< comma::int32 >
{
    static const format::types_enum value = format::int32;
    static const char* as_string() { return "i"; }
};

template <> struct format::type_to_enum< comma::uint32 >
{
    static const format::types_enum value = format::uint32;
    static const char* as_string() { return "ui"; }
};

template <> struct format::type_to_enum< comma::int64 >
{
    static const format::types_enum value = format::int64;
    static const char* as_string() { return "l"; }
};

template <> struct format::type_to_enum< comma::uint64 >
{
    static const format::types_enum value = format::uint64;
    static const char* as_string() { return "ul"; }
};

template <> struct format::type_to_enum< float >
{
    static const format::types_enum value = format::float_t;
    static const char* as_string() { return "f"; }
};

template <> struct format::type_to_enum< double >
{
    static const format::types_enum value = format::double_t;
    static const char* as_string() { return "d"; }
};

template <> struct format::type_to_enum< long double >
{
    static const format::types_enum value = format::double_t;
    static const char* as_string() { return "d"; }
};

template <> struct format::type_to_enum< boost::posix_time::ptime >
{
    static const format::types_enum value = format::time;
    static const char* as_string() { return "t"; }
};

template <> struct format::type_to_enum< std::string >
{
    static const format::types_enum value = format::fixed_string;
    static const char* as_string() { return "s"; }
};

template <> struct format::traits< boost::posix_time::ptime, format::long_time >
{
    static const types_enum type = format::long_time;
    static const unsigned int size = sizeof( comma::uint64 ) + sizeof( comma::uint32 );
    static const char* as_string() { return "lt"; }
    static boost::posix_time::ptime from_bin( const char* buf, std::size_t size = 12 );
    static void to_bin( const boost::posix_time::ptime& t, char* buf, std::size_t size = 12 );
};

template <> struct format::traits< boost::posix_time::ptime, format::time >
{
    static const types_enum type = format::time;
    static const unsigned int size = sizeof( comma::uint64 );
    static const char* as_string() { return "t"; }
    static boost::posix_time::ptime from_bin( const char* buf, std::size_t size = 8 );
    static void to_bin( const boost::posix_time::ptime& t, char* buf, std::size_t size = 8 );
};

template <> struct format::traits< std::string, format::fixed_string >
{
    static const types_enum type = format::fixed_string;
    static const char* as_string() { return type_to_enum< std::string >::as_string(); }
    static std::string from_bin( const char* buf, std::size_t size );
    static void to_bin( const std::string& t, char* buf, std::size_t size );
};

} } // namespace comma { namespace csv {

#endif // #ifndef COMMA_CSV_APPLICATIONS_FORMAT_HEADER_GUARD_
