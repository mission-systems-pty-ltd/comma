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

#ifndef COMMA_CSV_IMPL_FROMBINARY_HEADER_GUARD_
#define COMMA_CSV_IMPL_FROMBINARY_HEADER_GUARD_

#include <string.h>
#include <deque>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/type_traits.hpp>
#include <comma/base/types.h>
#include <comma/csv/format.h>
#include <comma/string/string.h>
#include <comma/visiting/visit.h>
#include <comma/visiting/while.h>
#include "./static_cast.h"

namespace comma { namespace csv { namespace impl {
    
/// visitor loading a struct from a csv file
/// see unit test for usage
class from_binary_
{
    public:
        /// constructor
        from_binary_( const std::vector< boost::optional< format::element > >& offsets
                  , const std::deque< bool >& optional
                  , const char* buf );
        
        /// apply
        template < typename K, typename T > void apply( const K& name, boost::optional< T >& value );
        
        /// apply
        template < typename K, typename T > void apply( const K& name, boost::scoped_ptr< T >& value );
        
        /// apply
        template < typename K, typename T > void apply( const K& name, boost::shared_ptr< T >& value );
        
        /// apply
        template < typename K, typename T > void apply( const K& name, T& value );
        
        /// apply to non-leaf elements
        template < typename K, typename T > void apply_next( const K& name, T& value );
        
        /// apply to leaf elements
        template < typename K, typename T > void apply_final( const K& name, T& value );
        
    private:
        const std::vector< boost::optional< format::element > >& offsets_;
        const std::deque< bool >& optional_;
        std::size_t optional_index;
        const char* buf_;
        std::size_t index_;
//         static void copy( boost::posix_time::ptime& v, const char* buf, std::size_t )
//         {
//             int64 seconds;
//             int32 nanoseconds;
//             ::memcpy( &seconds, buf, sizeof( int64 ) );
//             ::memcpy( &nanoseconds, buf + sizeof( int64 ), sizeof( int32 ) );
//             v = boost::posix_time::ptime(Timing::epoch, boost::posix_time::seconds( static_cast< long >( seconds ) ) + boost::posix_time::microseconds(nanoseconds/1000));
//         }
//         static void copy( std::string& v, const char* buf, std::size_t size ) // currently only for fixed size string
//         {
//             v = buf[ size - 1 ] == 0 ? std::string( buf ) : std::string( buf, size );
//         }
//         template < typename T >
//         static void copy( T& v, const char* buf, std::size_t size ) { ::memcpy( &v, buf, size ); }
};

inline from_binary_::from_binary_( const std::vector< boost::optional< format::element > >& offsets
                               , const std::deque< bool >& optional
                               , const char* buf )
    : offsets_( offsets )
    , optional_( optional )
    , optional_index( 0 )
    , buf_( buf )
    , index_( 0 )
{
}

template < typename K, typename T >
inline void from_binary_::apply( const K& name, boost::optional< T >& value ) // todo: watch performance
{
    if( !value && optional_[optional_index++] ) { value = T(); }
    if( value ) { this->apply( name, *value ); }
    else { ++index_; }
}

template < typename K, typename T >
inline void from_binary_::apply( const K& name, boost::scoped_ptr< T >& value ) // todo: watch performance
{
    if( !value && optional_[optional_index++] ) { value = T(); }
    if( value ) { this->apply( name, *value ); }
    else { ++index_; }
}

template < typename K, typename T >
inline void from_binary_::apply( const K& name, boost::shared_ptr< T >& value ) // todo: watch performance
{
    if( !value && optional_[optional_index++] ) { value = T(); }
    if( value ) { this->apply( name, *value ); }
    else { ++index_; }
}
                             
template < typename K, typename T >
inline void from_binary_::apply( const K& name, T& value )
{
    visiting::do_while<    !boost::is_fundamental< T >::value
                        && !boost::is_same< T, std::string >::value
                        && !boost::is_same< T, boost::posix_time::ptime >::value >::visit( name, value, *this );
}

template < typename K, typename T >
inline void from_binary_::apply_next( const K& name, T& value ) { comma::visiting::visit( name, value, *this ); }

inline void cast_( std::string& v, const std::string& s ) { v = s; }

inline void cast_( char& v, const std::string& s ) { if( !s.empty() ) { v = s[0]; } }

template < typename T > inline void cast_( T& v, const std::string& s ) // quick and dirty, watch performance
{
    const std::string& stripped = comma::strip( s, ' ' );
    if( !stripped.empty() ) { v = boost::lexical_cast< T >( stripped ); }
}

template < typename K, typename T >
inline void from_binary_::apply_final( const K&, T& value )
{
    //if( offsets_[ index_ ] ) { copy( value, buf_ + offsets_[ index_ ]->offset, offsets_[ index_ ]->size ); }
    if( offsets_[ index_ ] )
    {
        const char* buf = buf_ + offsets_[ index_ ]->offset;
        std::size_t size = offsets_[ index_ ]->size;
        format::types_enum type = offsets_[ index_ ]->type;
        if( type == format::traits< T >::type ) // quick path
        {
            value = format::traits< T >::from_bin( buf, size ); // copy( value, buf, size );
        }
        else
        {
            switch( type )
            {
                case format::int8: value = static_cast_impl< T >::value( format::traits< char >::from_bin( buf ) ); break;
                case format::uint8: value = static_cast_impl< T >::value( format::traits< unsigned char >::from_bin( buf ) ); break;
                case format::int16: value = static_cast_impl< T >::value( format::traits< comma::int16 >::from_bin( buf ) ); break;
                case format::uint16: value = static_cast_impl< T >::value( format::traits< comma::uint16 >::from_bin( buf ) ); break;
                case format::int32: value = static_cast_impl< T >::value( format::traits< comma::int32 >::from_bin( buf ) ); break;
                case format::uint32: value = static_cast_impl< T >::value( format::traits< comma::uint32 >::from_bin( buf ) ); break;
                case format::int64: value = static_cast_impl< T >::value( format::traits< comma::int64 >::from_bin( buf ) ); break;
                case format::uint64: value = static_cast_impl< T >::value( format::traits< comma::uint64 >::from_bin( buf ) ); break;
                case format::char_t: value = static_cast_impl< T >::value( format::traits< char >::from_bin( buf ) ); break;
                case format::float_t: value = static_cast_impl< T >::value( format::traits< float >::from_bin( buf ) ); break;
                case format::double_t: value = static_cast_impl< T >::value( format::traits< double >::from_bin( buf ) ); break;
                case format::time: value = static_cast_impl< T >::value( format::traits< boost::posix_time::ptime, format::time >::from_bin( buf ) ); break;
                case format::long_time: value = static_cast_impl< T >::value( format::traits< boost::posix_time::ptime, format::long_time >::from_bin( buf ) ); break;
                // quick and dirty: relax casting and see if it works...
                //case format::fixed_string: value = static_cast_impl< T >::value( format::traits< std::string >::from_bin( buf, size ) ); break;
                case format::fixed_string: cast_( value, format::traits< std::string >::from_bin( buf, size ) ); break;
            };
        }
    }
    ++index_;
}

} } } // namespace comma { namespace csv { namespace impl {

#endif // #ifndef COMMA_CSV_IMPL_FROMBINARY_HEADER_GUARD_
