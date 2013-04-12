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

#ifndef COMMA_CSV_SERIALIZATION_BINARYVISITOR_HEADER_GUARD_
#define COMMA_CSV_SERIALIZATION_BINARYVISITOR_HEADER_GUARD_

#include <deque>
#include <map>
#include <vector>
#include <boost/scoped_ptr.hpp>
#include <boost/type_traits.hpp>
#include <comma/csv/format.h>
#include <comma/string/string.h>
#include <comma/visiting/apply.h>
#include <comma/visiting/visit.h>
#include <comma/visiting/while.h>
#include <comma/xpath/xpath.h>

namespace comma { namespace csv { namespace impl {

/// visitor loading column names to be used later
/// in loading a struct from a binary csv file
/// see unit test for usage
class binary_visitor
{
    public:
        /// constructor
        //binary_visitor( const std::string& format, const std::string& column_names = "", bool full_path_as_name = true )
        binary_visitor( const csv::format& format, const std::string& column_names = "", bool full_path_as_name = true )
            : format_( format )
            , full_path_as_name_( full_path_as_name )
        {
            if( column_names == "" ) { return; }
            std::vector< std::string > v = split( column_names, ',' );
            for( std::size_t i = 0; i < v.size(); ++i ) { map_[ v[i] ] = i; }
        }
        
        template < typename K, typename T >
        void apply( const K& name, const boost::optional< T >& value ) // breaks windows compiler: { apply_optional( name, value ); }
        {
            empty_.push_back( true );
            if( value ) { apply( name, *value ); } else { T v; apply( name, v ); }
            optional_.push_back( !empty_.back() );
            empty_.pop_back();
        }
        
        template < typename K, typename T >
        void apply( const K& name, const boost::scoped_ptr< T >& value ) // breaks windows compiler: { apply_optional( name, value ); }
        {
            empty_.push_back( true );
            if( value ) { apply( name, *value ); } else { T v; apply( name, v ); }
            optional_.push_back( !empty_.back() );
            empty_.pop_back();
        }
        
        template < typename K, typename T >
        void apply( const K& name, const boost::shared_ptr< T >& value ) // breaks windows compiler: { apply_optional( name, value ); }
        {
            empty_.push_back( true );
            if( value ) { apply( name, *value ); } else { T v; apply( name, v ); }
            optional_.push_back( !empty_.back() );
            empty_.pop_back();
        }
        
        /// apply
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
        void apply_final( const K& key, const T& t )
        {
			(void)key;
            std::map< std::string, std::size_t >::const_iterator it = map_.find( full_path_as_name_ ? xpath_.to_string() : xpath_.elements.back().to_string() );
            optional_element o;
            if( map_.empty() || it != map_.end() )
            {
                for( std::size_t i = 0; i < empty_.size(); ++i ) { empty_[i] = false; }
                o = offset( t, it->second );
            }
            offsets_.push_back( o );
        }
        
        /// a convenience type
        typedef boost::optional< format::element > optional_element;
        
        /// return field offsets
        const std::vector< optional_element >& offsets() const { return offsets_; }
        
        /// return flags, which are true for optional values that are present
        const std::deque< bool >& optional() const { return optional_; }
        
    private:
        std::map< std::string, std::size_t > map_;
        csv::format format_;
        bool full_path_as_name_;
        xpath xpath_;
        std::vector< optional_element > offsets_;
        std::deque< bool > empty_;
        std::deque< bool > optional_;
        const xpath& append( std::size_t index ) { xpath_.elements.back().index = index; return xpath_; }
        const xpath& append( const char* name ) { xpath_ /= xpath::element( name ); return xpath_; }
        const xpath& trim( std::size_t ) { xpath_.elements.back().index = boost::optional< std::size_t >(); return xpath_; }
        const xpath& trim( const char* ) { xpath_ = xpath_.head(); return xpath_; }
//         optional_element offset( const std::string&, std::size_t index )
//         {
//             std::pair< unsigned int, unsigned int > p = format_.index( index + string_sTotalSize );
//             if( p.second != 0 || format_.types()[ p.first ] != format::char_t ) { COMMA_THROW( comma::exception, "expected fixed-size string in " << format_.string() << " as element " << index << " (e.g. %4c for string of size 4)" ); }
//             string_sTotalSize += format_.offsets()[ p.first ].count;
//             return optional_element( std::make_pair( format_.offsets()[ p.first ].offset, format_.offsets()[ p.first ].count ) );
//         }
//         template < typename T > optional_element offset( const T&, std::size_t index ) { return format_.offset( index + string_sTotalSize ); }
        template < typename T > optional_element offset( const T&, std::size_t index ) { return format_.offset( index ); }
        
//        template < typename K, typename T, template < typename > class optional_element >
//        void apply_optional( const K& name, const optional_element< T >& value )
//        {
//            empty_.push_back( true );
//            if( value ) { apply( name, *value ); } else { T v; apply( name, v ); }
//            optional_.push_back( !empty_.back() );
//            empty_.pop_back();
//        }
};

} } } // namespace comma { namespace csv { namespace impl {

#endif // #ifndef COMMA_CSV_SERIALIZATION_BINARYVISITOR_HEADER_GUARD_
