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

#ifndef COMMA_CSV_IMPL_FROMASCII_HEADER_GUARD_
#define COMMA_CSV_IMPL_FROMASCII_HEADER_GUARD_

#include <deque>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/type_traits.hpp>
#include "../../base/exception.h"
#include "../../visiting/visit.h"
#include "../../visiting/while.h"

namespace comma { namespace csv { namespace impl {

/// visitor loading a struct from a csv file
/// see unit test for usage
class from_ascii_
{
    public:
        /// constructor
        from_ascii_( const std::vector< boost::optional< std::size_t > >& indices
                  , const std::deque< bool >& optional
                  , const std::vector< std::string >& line );

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
        const std::vector< boost::optional< std::size_t > >& indices_;
        const std::deque< bool >& optional_;
        const std::vector< std::string >& row_;
        std::size_t index_;
        std::size_t optional_index;
        static void lexical_cast_( char& v, const std::string& s ) { v = s.at( 0 ) == '\'' && s.at( 2 ) == '\'' && s.length() == 3 ? s.at( 1 ) : static_cast< char >( boost::lexical_cast< int >( s ) ); }
        static void lexical_cast_( unsigned char& v, const std::string& s ) { v = s.at( 0 ) == '\'' && s.at( 2 ) == '\'' && s.length() == 3 ? s.at( 1 ) : static_cast< unsigned char >( boost::lexical_cast< unsigned int >( s ) ); }
        static void lexical_cast_( boost::posix_time::ptime& v, const std::string& s ) { if( s.empty() ) { return; } try { v = boost::posix_time::from_iso_string( s ); } catch( ... ) { v = boost::posix_time::not_a_date_time; } }
        static void lexical_cast_( std::string& v, const std::string& s ) { v = comma::strip( s, "\"" ); }
        static void lexical_cast_( bool& v, const std::string& s ) { if( s.empty() ) { return; } v = static_cast< bool >( boost::lexical_cast< unsigned int >( s ) ); }
        template < typename T >
        static void lexical_cast_( T& v, const std::string& s ) { if( s.empty() ) { return; } v = boost::lexical_cast< T >( s ); }
};

inline from_ascii_::from_ascii_( const std::vector< boost::optional< std::size_t > >& indices
                           , const std::deque< bool >& optional
                           , const std::vector< std::string >& line )
    : indices_( indices )
    , optional_( optional )
    , row_( line )
    , index_( 0 )
    , optional_index( 0 )
{
}

template < typename K, typename T >
inline void from_ascii_::apply( const K& name, boost::optional< T >& value ) // todo: watch performance
{
    if( !value && optional_[optional_index++] ) { value = T(); }
    if( value ) { this->apply( name, *value ); }
    else { ++index_; }
}

template < typename K, typename T >
inline void from_ascii_::apply( const K& name, boost::scoped_ptr< T >& value ) // todo: watch performance
{
    if( !value && optional_[optional_index++] ) { value = T(); }
    if( value ) { this->apply( name, *value ); }
    else { ++index_; }
}

template < typename K, typename T >
inline void from_ascii_::apply( const K& name, boost::shared_ptr< T >& value ) // todo: watch performance
{
    if( !value && optional_[optional_index++] ) { value = T(); }
    if( value ) { this->apply( name, *value ); }
    else { ++index_; }
}

template < typename K, typename T >
inline void from_ascii_::apply( const K& name, T& value )
{
    visiting::do_while<    !boost::is_fundamental< T >::value
                        && !boost::is_same< T, std::string >::value
                        && !boost::is_same< T, boost::posix_time::ptime >::value >::visit( name, value, *this );
}

template < typename K, typename T >
inline void from_ascii_::apply_next( const K& name, T& value ) { comma::visiting::visit( name, value, *this ); }

template < typename K, typename T >
inline void from_ascii_::apply_final( const K& key, T& value )
{
	(void)key;
    if( indices_[ index_ ] )
    {
        std::size_t i = *indices_[ index_ ];
        if( i >= row_.size() ) { COMMA_THROW( comma::exception, "got column index " << i << ", for " << row_.size() << " column(s) in line: \"" << join( row_, ',' ) << "\"" ); }
        const std::string& s = row_[i];
        if( !s.empty() ) { lexical_cast_( value, s ); }
    }
    ++index_;
}

} } } // namespace comma { namespace csv { namespace impl {

#endif // #ifndef COMMA_CSV_IMPL_FROMASCII_HEADER_GUARD_
