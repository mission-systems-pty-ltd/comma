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


/// @author vsevolod vlaskine

#ifndef COMMA_CSV_IMPL_TONAMES_H_
#define COMMA_CSV_IMPL_TONAMES_H_

#include <sstream>
#include <boost/optional.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/type_traits.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <comma/visiting/traits.h>
#include <comma/visiting/visit.h>
#include <comma/visiting/while.h>
#include <comma/xpath/xpath.h>

namespace comma { namespace csv { namespace impl {

/// visitor converting a structure into comma-separated xpaths
/// @todo implement handling vectors, arrays, etc
class to_names
{
    public:
        /// constructor
        to_names( bool full_path_as_name = true );
        
        /// constructor
        to_names( const xpath& root, bool full_path_as_name );
        
        /// traverse
        template < typename K, typename T >
        void apply( const K& name, const boost::optional< T >& value );
        
        /// traverse
        template < typename K, typename T >
        void apply( const K& name, const boost::scoped_ptr< T >& value );
        
        /// traverse
        template < typename K, typename T >
        void apply( const K& name, const boost::shared_ptr< T >& value );
        
        /// traverse
        template < typename K, typename T >
        void apply( const K& name, const T& value );
        
        /// traverse
        template < typename K, typename T >
        void apply_next( const K& name, const T& value );
        
        /// output a non-string type
        template < typename K, typename T >
        void apply_final( const K& name, const T& value );
        
        /// return string
        const std::vector< std::string >& operator()() const;
        
    private:
        bool full_path_as_name_;
        xpath xpath_;
        xpath root_;
        boost::optional< std::size_t > index_;
        std::vector< std::string > names_;
        const xpath& append( std::size_t index ) { xpath_.elements.back().index = index; return xpath_; }
        const xpath& append( const char* name ) { xpath_ /= xpath::element( name ); return xpath_; }
        const xpath& trim( std::size_t ) { xpath_.elements.back().index = boost::optional< std::size_t >(); return xpath_; }
        const xpath& trim( const char* ) { xpath_ = xpath_.head(); return xpath_; }
};

inline to_names::to_names( bool full_path_as_name ) : full_path_as_name_( full_path_as_name ) {}

inline to_names::to_names( const xpath& root, bool full_path_as_name ) : full_path_as_name_( full_path_as_name ), root_( root ) {}

template < typename K, typename T >
inline void to_names::apply( const K& name, const boost::optional< T >& value )
{
    apply( name, value ? *value : T() );
}

template < typename K, typename T >
inline void to_names::apply( const K& name, const boost::scoped_ptr< T >& value )
{
    if( value ) { apply( name, *value ); } else { T v; apply( name, v ); }
}

template < typename K, typename T >
inline void to_names::apply( const K& name, const boost::shared_ptr< T >& value )
{
    if( value ) { apply( name, *value ); } else { T v; apply( name, v ); }
}

template < typename K, typename T >
inline void to_names::apply( const K& name, const T& value )
{
    visiting::do_while<    !boost::is_fundamental< T >::value
                        && !boost::is_same< T, std::string >::value
                        && !boost::is_same< T, boost::posix_time::ptime >::value >::visit( name, value, *this );
}

template < typename K, typename T >
inline void to_names::apply_next( const K& name, const T& value )
{
    append( name );
    comma::visiting::visit( name, value, *this );
    trim( name );
}

template < typename K, typename T >
inline void to_names::apply_final( const K& name, const T& )
{
    append( name );
    if( xpath_ <= root_ ) { names_.push_back( full_path_as_name_ ? xpath_.to_string() : xpath_.elements.back().to_string() ); }
    trim( name );
}

inline const std::vector< std::string >& to_names::operator()() const { return names_; }

} } } // namespace comma { namespace csv { namespace impl {

#endif // COMMA_CSV_IMPL_TONAMES_H_
