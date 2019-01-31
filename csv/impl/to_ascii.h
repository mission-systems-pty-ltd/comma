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

#ifndef COMMA_CSV_IMPL_TOASCII_HEADER_GUARD_
#define COMMA_CSV_IMPL_TOASCII_HEADER_GUARD_

#include <vector>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/type_traits.hpp>
#include "../../base/exception.h"
#include "../../string/string.h"
#include "../../visiting/visit.h"
#include "../../visiting/while.h"

namespace comma { namespace csv { namespace impl {

/// visitor loading a struct from a csv file
/// see unit test for usage
class to_ascii
{
    public:
        /// constructor
        to_ascii( const std::vector< boost::optional< std::size_t > >& indices
                , std::vector< std::string >& line
                , boost::optional< char > quote );

        /// apply
        template < typename K, typename T > void apply( const K& name, const boost::optional< T >& value );

        /// apply
        template < typename K, typename T > void apply( const K& name, const boost::scoped_ptr< T >& value );

        /// apply
        template < typename K, typename T > void apply( const K& name, const boost::shared_ptr< T >& value );

        /// apply
        template < typename K, typename T >
        void apply( const K& name, const T& value );

        /// apply to non-leaf elements
        template < typename K, typename T >
        void apply_next( const K& name, const T& value );

        /// apply to leaf elements
        template < typename K, typename T >
        void apply_final( const K& name, const T& value );

        /// set precision
        void precision( unsigned int p ) { precision_ = p; }

    private:
        const std::vector< boost::optional< std::size_t > >& indices_;
        std::vector< std::string >& row_;
        std::size_t index_;
        boost::optional< unsigned int > precision_;
        boost::optional< char > quote_;

        std::string as_string_( const boost::posix_time::ptime& v ) { return to_iso_string( v ); }
        std::string as_string_( const std::string& v ) { return quote_ ? *quote_ + v + *quote_ : v; } // todo: escape/unescape
        // todo: better output semantics for char/unsigned char
        std::string as_string_( const char& v ) { std::ostringstream oss; oss << static_cast< int >( v ); return oss.str(); }
        std::string as_string_( const unsigned char& v ) { std::ostringstream oss; oss << static_cast< unsigned int >( v ); return oss.str(); }
        template < typename T >
        std::string as_string_( T v )
        {
            std::ostringstream oss;
            if( precision_ ) { oss.precision( *precision_ ); }
            oss << v;
            return oss.str();
        }
};

inline to_ascii::to_ascii( const std::vector< boost::optional< std::size_t > >& indices
                         , std::vector< std::string >& line
                         , boost::optional< char > quote )
    : indices_( indices )
    , row_( line )
    , index_( 0 )
    , quote_( quote )
{
}

template < typename K, typename T >
inline void to_ascii::apply( const K& name, const boost::optional< T >& value )
{
    if( value ) { apply( name, *value ); }
}

template < typename K, typename T >
inline void to_ascii::apply( const K& name, const boost::scoped_ptr< T >& value )
{
    if( value ) { apply( name, *value ); }
}

template < typename K, typename T >
inline void to_ascii::apply( const K& name, const boost::shared_ptr< T >& value )
{
    if( value ) { apply( name, *value ); }
}

template < typename K, typename T >
inline void to_ascii::apply( const K& name, const T& value )
{
    visiting::do_while<    !boost::is_fundamental< T >::value
                        && !boost::is_same< T, std::string >::value
                        && !boost::is_same< T, boost::posix_time::ptime >::value >::visit( name, value, *this );
}

template < typename K, typename T >
inline void to_ascii::apply_next( const K& name, const T& value ) { comma::visiting::visit( name, value, *this ); }

template < typename K, typename T >
inline void to_ascii::apply_final( const K&, const T& value )
{
    if( indices_[ index_ ] )
    {
        std::size_t i = *indices_[ index_ ];
        if( i >= row_.size() ) { COMMA_THROW( comma::exception, "got column index " << i << ", for " << row_.size() << " columns in row " << join( row_, ',' ) ); }
        row_[i] = as_string_( value );
    }
    ++index_;
}

} } } // namespace comma { namespace csv { namespace impl {

#endif // #ifndef COMMA_CSV_IMPL_TOASCII_HEADER_GUARD_
