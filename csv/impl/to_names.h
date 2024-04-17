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

#pragma once

#include <memory>
#if __cplusplus >= 201703L
#include <optional>
#endif // #if __cplusplus >= 201703L
#include <sstream>
#include <boost/optional.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/type_traits.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "../../visiting/traits.h"
#include "../../visiting/visit.h"
#include "../../visiting/while.h"
#include "../../xpath/xpath.h"

namespace comma { namespace csv { namespace impl {

/// visitor converting a structure into comma-separated xpaths
/// @todo implement handling vectors, arrays, etc
class to_names
{
    public:
        to_names( bool full_path_as_name = true ): _full_path_as_name( full_path_as_name ) {}
        
        to_names( const xpath& root, bool full_path_as_name ): _full_path_as_name( full_path_as_name ), _root( root ) {}
        
        template < typename K, typename T >
        void apply( const K& name, const boost::optional< T >& value ) { apply( name, value ? *value : T() ); }

        #if __cplusplus >= 201703L
        template < typename K, typename T >
        void apply( const K& name, const std::optional< T >& value ) { apply( name, value ? *value : T() ); }
        #endif // #if __cplusplus >= 201703L
        
        template < typename K, typename T >
        void apply( const K& name, const boost::scoped_ptr< T >& value );
        
        template < typename K, typename T >
        void apply( const K& name, const boost::shared_ptr< T >& value );

        template < typename K, typename T >
        void apply( const K& name, const std::unique_ptr< T >& value );
        
        template < typename K, typename T >
        void apply( const K& name, const T& value );
        
        template < typename K, typename T >
        void apply_next( const K& name, const T& value );
        
        template < typename K, typename T >
        void apply_final( const K& name, const T& value );
        
        const std::vector< std::string >& operator()() const { return _names; }
        
    private:
        bool _full_path_as_name;
        xpath _xpath;
        xpath _root;
        boost::optional< std::size_t > _index;
        std::vector< std::string > _names;
        const xpath& _append( std::size_t index ) { _xpath.elements.back().index = index; return _xpath; }
        const xpath& _append( const char* name ) { _xpath /= xpath::element( name ); return _xpath; }
        const xpath& _append( const std::string& name ) { _xpath /= xpath::element( name ); return _xpath; }
        const xpath& _trim( std::size_t ) { _xpath.elements.back().index = boost::optional< std::size_t >(); return _xpath; }
        const xpath& _trim( const char* ) { _xpath = _xpath.head(); return _xpath; }
        const xpath& _trim( const std::string& ) { _xpath = _xpath.head(); return _xpath; }
};

template < typename K, typename T >
inline void to_names::apply( const K& name, const boost::scoped_ptr< T >& value ) { if( value ) { apply( name, *value ); } else { T v; apply( name, v ); } }

template < typename K, typename T >
inline void to_names::apply( const K& name, const boost::shared_ptr< T >& value ) { if( value ) { apply( name, *value ); } else { T v; apply( name, v ); } }

template < typename K, typename T >
inline void to_names::apply( const K& name, const std::unique_ptr< T >& value ) { if( value ) { apply( name, *value ); } else { T v; apply( name, v ); } }

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
    _append( name );
    comma::visiting::visit( name, value, *this );
    _trim( name );
}

template < typename K, typename T >
inline void to_names::apply_final( const K& name, const T& )
{
    _append( name );
    if( _xpath <= _root ) { _names.push_back( _full_path_as_name ? _xpath.to_string() : _xpath.elements.back().to_string() ); }
    _trim( name );
}

} } } // namespace comma { namespace csv { namespace impl {
