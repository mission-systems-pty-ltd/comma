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

#ifndef COMMA_CSV_SERIALIZATION_ASCIIVISITOR_HEADER_GUARD_
#define COMMA_CSV_SERIALIZATION_ASCIIVISITOR_HEADER_GUARD_

#include <deque>
#include <map>
#include <vector>
#include <boost/scoped_ptr.hpp>
#include <boost/type_traits.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <comma/string/string.h>
#include <comma/visiting/apply.h>
#include <comma/visiting/visit.h>
#include <comma/visiting/while.h>
#include <comma/xpath/xpath.h>

namespace comma { namespace csv { namespace impl {

/// visitor loading column names to be used later
/// in loading a struct from a csv file
/// see unit test for usage
class asciiVisitor
{
    public:
        asciiVisitor( const std::string& column_names = "", bool full_path_as_name = true )
            : full_path_as_name_( full_path_as_name )
            , size_( 0 )
        {
            if( column_names == "" ) { return; }
            std::vector< std::string > v = split( column_names, ',' );
            for( std::size_t i = 0; i < v.size(); ++i ) { map_[ v[i] ] = i; }
        }

        template < typename K, typename T >
        void apply( const K& name, const boost::optional< T >& value ) { apply_optional( name, value ); }

        template < typename K, typename T >
        void apply( const K& name, const boost::scoped_ptr< T >& value ) { apply_optional( name, value ); }

        template < typename K, typename T >
        void apply( const K& name, const boost::shared_ptr< T >& value ) { apply_optional( name, value ); }

        template < typename K, typename T >
        void apply( const K& name, const T& value )
        {
            append( name );
            visiting::do_while<    !boost::is_fundamental< T >::value
                                && !boost::is_same< T, std::string >::value
                                && !boost::is_same< T, boost::posix_time::ptime >::value >::visit( name, value, *this );
            trim( name );
        }

        /// apply to non-leaf elements
        template < typename K, typename T >
        void apply_next( const K& name, const T& value ) { comma::visiting::visit( name, value, *this ); }

        /// apply to leaf elements
        template < typename K, typename T >
        void apply_final( const K&, const T& )
        {
            std::map< std::string, std::size_t >::const_iterator it = map_.find( full_path_as_name_ ? xpath_.to_string() : xpath_.elements.back().to_string() );
            boost::optional< std::size_t > index;
            if( map_.empty() || it != map_.end() )
            {
                for( std::size_t i = 0; i < empty_.size(); ++i ) { empty_[i] = false; }
                index = boost::optional< std::size_t >( it->second );
                ++size_;
            }
            indices_.push_back( index );
        }

        /// return field indices
        const std::vector< boost::optional< std::size_t > >& indices() const { return indices_; }

        /// return flags, which are true for optional values that are present
        const std::deque< bool >& optional() const { return optional_; }

        /// return number of columns that will be visited
        std::size_t size() const { return size_; }

    private:
        bool full_path_as_name_;
        std::map< std::string, std::size_t > map_;
        xpath xpath_;
        std::vector< boost::optional< std::size_t > > indices_;
        std::size_t size_;
        std::deque< bool > empty_;
        std::deque< bool > optional_;
        const xpath& append( std::size_t index ) { xpath_.elements.back().index = index; return xpath_; }
        const xpath& append( const char* name ) { xpath_ /= xpath::element( name ); return xpath_; }
        const xpath& trim( std::size_t ) { xpath_.elements.back().index = boost::optional< std::size_t >(); return xpath_; }
        const xpath& trim( const char* ) { xpath_ = xpath_.head(); return xpath_; }
        template < typename K, typename T, template < typename > class optional_element >
        void apply_optional( const K& name, const optional_element< T >& value )
        {
            empty_.push_back( true );
            if( value ) { this->apply( name, *value ); } else { T v; this->apply( name, v ); }
            optional_.push_back( !empty_.back() );
            empty_.pop_back();
        }
};

} } } // namespace comma { namespace csv { namespace impl {

#endif // #ifndef COMMA_CSV_SERIALIZATION_ASCIIVISITOR_HEADER_GUARD_
