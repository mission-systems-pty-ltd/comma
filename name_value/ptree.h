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


/// @author cedric wohlleber
/// @author vsevolod vlaskine

#ifndef COMMA_NAME_VALUE_PTREE_H_
#define COMMA_NAME_VALUE_PTREE_H_

#include <iostream>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/unordered_set.hpp>
#include <comma/base/exception.h>
#include <comma/base/types.h>
#include <comma/string/string.h>
#include <comma/xpath/xpath.h>
#include <comma/visiting/visit.h>
#include <comma/visiting/while.h>

namespace comma {

struct property_tree // quick and dirty
{
    /// read as name-value from input stream
    /// @todo currently only line-based input supported
    static void from_name_value( std::istream& is, boost::property_tree::ptree& ptree, char equal_sign = '=', char delimiter = ',' );

    /// write as name-value to output stream
    static void to_name_value( std::ostream& os, const boost::property_tree::ptree& ptree, bool indented = true, char equal_sign = '=', char delimiter = ',' );

    /// convert name=value-style string into boost parameter tree
    static boost::property_tree::ptree from_name_value_string( const std::string& s, char equal_sign = '=', char delimiter = ',' );

    /// convert boost parameter tree into name=value-style string
    static std::string to_name_value_string( const boost::property_tree::ptree& ptree, bool indented = true, char equal_sign = '=', char delimiter = ',' );

    /// disabled: do not show path indices for array items, with_brackets: x[0]/y/z[0]=1 and without_brackets: e.g. x/0/y/z/1=1
    enum path_mode { disabled, with_brackets, without_brackets  };

    /// write as path-value to output stream
    static void to_path_value( std::ostream& os, const boost::property_tree::ptree& ptree, path_mode indices_mode = disabled, char equal_sign = '=', char delimiter = ',', const xpath& root = xpath() );

    /// convert boost parameter tree into path=value-style string (equal sign and delimiter have to be escaped)
    static std::string to_path_value_string( const boost::property_tree::ptree& ptree, path_mode mode=disabled, char equal_sign = '=', char delimiter = ',' );
    
    /// put an xpath like a/b[5]/c/d[3]=4 into ptree
    static void put( boost::property_tree::ptree& ptree, const xpath& path, const std::string& value );
    
    /// get value as string from an xpath like a/b[5]/c/d[3]=4 on ptree
    static boost::optional< std::string > get( boost::property_tree::ptree& ptree, const xpath& path );

    /// for path-value strings only
    /// read as path-value from string; enum specifies how to treat repeated paths (foo="bar"; foo="blah";)
    struct path_value
    {
        enum check_repeated_paths
        { 
            /// no check at all
              no_check
            /// if multiple entries with the same path, take the last entry
            , take_last
            /// expect unique paths on input, can overwrite existing values in ptree
            , unique_input
            /// expect unique paths on input, cannot overwrite existing values in ptree
            , no_overwrite
        };
    };

    /// convert path-value-style string into boost parameter tree
    static boost::property_tree::ptree from_path_value_string( const std::string& s, char equal_sign = '=', char delimiter = ',', path_value::check_repeated_paths check_type = path_value::no_check, bool use_index = false );
    static boost::property_tree::ptree& from_path_value_string( boost::property_tree::ptree& ptree, const std::string& s, char equal_sign, char delimiter, path_value::check_repeated_paths check_type = path_value::no_check, bool use_index = false );

    /// read as path-value from input stream
    static void from_path_value( std::istream& is, boost::property_tree::ptree& ptree,  path_value::check_repeated_paths check_type = path_value::no_check, char equal_sign = '=', char delimiter = ',', bool use_index = false );

    /// guess format and read boost property tree from stream (a wrapper for from_unknown_seekable that buffers input from a non-seekable stream)
    static void from_unknown( std::istream& stream, boost::property_tree::ptree& ptree, path_value::check_repeated_paths check_type = path_value::no_check, char equal_sign = '=', char delimiter = ',', bool use_index = false  );

    /// guess format and read boost property tree from a seekable stream (pipe or terminal input is not accepted)
    static void from_unknown_seekable( std::istream& stream, boost::property_tree::ptree& ptree, path_value::check_repeated_paths check_type = path_value::no_check, char equal_sign = '=', char delimiter = ',', bool use_index = false  );

    template < template < typename > class Traits = comma::visiting::traits >
    class from
    {
        public:
            /// constructor
            /// @param ptree: property tree for the structure to fill
            /// @param root: path to the root of the subtree to visit
            /// @param branch: path to the subtree to visit (i.e. other branches will be pruned)
            from( const boost::property_tree::ptree& ptree )
                : ptree_( ptree )
                , cur_( ptree )
                , permissive_( false )
            {
            }
            from( const boost::property_tree::ptree& ptree, bool permissive )
                : ptree_( ptree )
                , cur_( ptree )
                , permissive_( permissive )
            {
            }
            from( const boost::property_tree::ptree& ptree, const char* root, bool permissive = false )
                : ptree_( ptree )
                , cur_( ptree_.get_child_optional( xpath( root ).to_string( '.' ) ) )
                , permissive_( permissive )
            {
            }
            from( const boost::property_tree::ptree& ptree, const xpath& root, bool permissive = false )
                : ptree_( ptree )
                , cur_( ptree_.get_child_optional( root.to_string( '.' ) ) )
                , permissive_( permissive )
            {
            }

            //ptree_visitor( const boost::property_tree::ptree& ptree, const xpath& root, const xpath& branch, bool permissive = false ) : ptree_( ptree ), cur_( &ptree ), path_( root ), branch_( branch ), permissive_( permissive ) {}

            /// apply
            template < typename K, typename T >
            void apply( const K& key, T& value )
            {
                visiting::do_while<    !boost::is_fundamental< T >::value
                                    && !boost::is_same< T, boost::posix_time::ptime >::value
                                    && !boost::is_same< T, std::string >::value >::visit( key, value, *this );
            }

            /// apply on boost optional
            template < typename K, typename T >
            void apply_next( const K& name, boost::optional< T >& value )
            {
                if( !cur_ || cur_->find( name ) == cur_->not_found() ) { return; }
                if( !value ) { value = T(); }
                apply( name, *value );
            }

            /// apply to vector
            template < typename K, typename T, typename A >
            void apply_next( const K& key, std::vector< T, A >& value )
            {
                std::string name = boost::lexical_cast< std::string >( key );
                boost::optional< const boost::property_tree::ptree& > t = cur_ && !name.empty() ? cur_->get_child_optional( name ) : cur_;
                if( t )
                {
                    const boost::property_tree::ptree& parent = *cur_;
                    value.resize( t->size() );
                    std::size_t i = 0;
                    for( boost::property_tree::ptree::const_assoc_iterator j = t->ordered_begin(); j != t->not_found(); ++j, ++i )
                    {
                        cur_ = j->second;
                        //std::size_t index = j->first == "" ? i : boost::lexical_cast< std::size_t >( j->first ); // way quick and dirty
                        //if( index >= t->size() ) { COMMA_THROW( comma::exception, "expected index less than " << t->size() << "; got: " << index ); }
                        visiting::do_while<    !boost::is_fundamental< T >::value
                                            && !boost::is_same< T, boost::posix_time::ptime >::value
                                            && !boost::is_same< T, std::string >::value >::visit( "", value[i], *this );
                    }
                    cur_ = parent;
                }
                else if( !permissive_ )
                {
                    COMMA_THROW( comma::exception, "key " << key << " not found" );
                }
            }

            /// apply to map
            template < typename K, typename L, typename T, typename A >
            void apply_next( const K& key, std::map< L, T, A >& value )
            {
                std::string name = boost::lexical_cast< std::string >( key );
                boost::optional< const boost::property_tree::ptree& > t = cur_ && !name.empty() ? cur_->get_child_optional( name ) : cur_;
                if( t )
                {
                    const boost::property_tree::ptree& parent = *cur_;
                    for( boost::property_tree::ptree::const_assoc_iterator j = t->ordered_begin(); j != t->not_found(); ++j )
                    {
                        cur_ = j->second;
                        visiting::do_while<    !boost::is_fundamental< T >::value
                                            && !boost::is_same< T, boost::posix_time::ptime >::value
                                            && !boost::is_same< T, std::string >::value >::visit( "", value[ boost::lexical_cast< L >( j->first ) ], *this );
                    }
                    cur_ = parent;
                }
                else if( !permissive_ )
                {
                    COMMA_THROW( comma::exception, "key not found: " << key );
                }
            }

            /// apply to non-leaf elements
            template < typename K, typename T >
            void apply_next( const K& key, T& value )
            {
                if( !cur_ )
                {
                    if( permissive_ ) { return; }
                    COMMA_THROW( comma::exception, "key " << key << " not found" );
                }
                const boost::property_tree::ptree& parent = *cur_;
                std::string name = boost::lexical_cast< std::string >( key );
                if( !name.empty() ) { cur_ = cur_->get_child_optional( name ); }
                Traits< T >::visit( key, value, *this );
                cur_ = parent;
            }

            /// apply to leaf elements
            template < typename K, typename T >
            void apply_final( const K& key, T& value )
            {
                boost::optional< T > v;
                value_( boost::lexical_cast< std::string >( key ), v );
                if( v ) { value = *v; }
                else if( !permissive_ ) { COMMA_THROW( comma::exception, "key not found: " << key ); }
            }

        private:
            const boost::property_tree::ptree& ptree_;
            boost::optional< const boost::property_tree::ptree& > cur_;
            const bool permissive_;
            void value_( const std::string& name, boost::optional< boost::posix_time::ptime >& v ) // quick and dirty, imlement traits instead
            {
                if( !cur_ ) { return; }
                boost::optional< std::string > s = name.empty() ? cur_->get_value_optional< std::string >() : cur_->get_optional< std::string >( name );
                if( !s ) { s = cur_->get_optional< std::string >( "<xmlattr>." + name ); }
                if( s ) { v = boost::posix_time::from_iso_string( *s ); }
            }
            template < typename T > void value_( const std::string& name, boost::optional< T >& v )
            {
                if( !cur_ ) { return; }
                v = name.empty() ? cur_->get_value_optional< T >() : cur_->get_optional< T >( name );
                if( !v ) { v = cur_->get_optional< T >( "<xmlattr>." + name ); }
            }
    };
};

typedef property_tree::from<> from_ptree;

/// @todo redesign like from_ptree
class to_ptree
{
    public:
        /// constructor
        /// @param ptree: property tree for the structure to fill
        /// @param root: path to the root of the subtree to visit
        /// @param branch: path to the subtree to visit (i.e. other branches will be pruned)
        to_ptree( boost::property_tree::ptree& ptree, const xpath& root = xpath(), const xpath& branch = xpath() ) : ptree_( ptree ), path_( root ), branch_( branch ) {}

        /// constructor
        to_ptree( boost::property_tree::ptree& ptree, const char* root ) : ptree_( ptree ), path_( root ) {}

        /// apply_next on boost optional
        template < typename K, typename T >
        void apply_next( const K& name, const boost::optional< T >& value )
        {
            if( value )
            {
                visiting::do_while<    !boost::is_fundamental< T >::value
                                    && !boost::is_same< T, boost::posix_time::ptime >::value
                                    && !boost::is_same< T, std::string >::value >::visit( name, *value, *this );
            }
        }

        /// apply
        template < typename K, typename T, typename A >
        void apply( const K& name, const std::vector< T, A >& value )
        {
            if( !( path_ <= branch_ ) ) { return; } // visit, only if on the branch
            append_( name );
            for( unsigned int i = 0; i < value.size(); ++i )
            {
                append_( boost::lexical_cast< std::string >( i ).c_str() );
                visiting::do_while<    !boost::is_fundamental< T >::value
                                    && !boost::is_same< T, boost::posix_time::ptime >::value
                                    && !boost::is_same< T, std::string >::value >::visit( i, value[i], *this );
                trim_();
            }
            trim_( name );
        }

        /// apply
        template < typename K, typename T >
        void apply( const K& name, const T& value )
        {
            if( !( path_ <= branch_ ) ) { return; } // visit, only if on the branch
            const std::string& s = boost::lexical_cast< std::string >( name );
            append_( &s[0] );
            visiting::do_while<    !boost::is_fundamental< T >::value
                                && !boost::is_same< T, boost::posix_time::ptime >::value
                                && !boost::is_same< T, std::string >::value >::visit( name, value, *this );
            trim_( &s[0] );
        }

        /// apply to non-leaf elements
        template < typename K, typename T >
        void apply_next( const K& name, const T& value )
        {
            comma::visiting::visit( name, value, *this );
        }

        /// apply to leaf elements
        template < typename K, typename T >
        void apply_final( const K&, const T& value ) { ptree_.put( path_.to_string( '.' ), value_( value ) ); }

    private:
        boost::property_tree::ptree& ptree_;
        xpath path_;
        xpath branch_;
        // quick and dirty
        //const xpath& append_( std::size_t index ) { path_.elements.back().index = index; return path_; }
        const xpath& append_( std::size_t index ) { path_ /= xpath::element( boost::lexical_cast< std::string >( index ) ); return path_; }
        const xpath& append_( const char* name ) { path_ /= xpath::element( name ); return path_; }
        //const xpath& trim_( std::size_t size ) { (void) size; path_.elements.back().index = boost::optional< std::size_t >(); return path_; }
        const xpath& trim_( std::size_t size ) { ( void )( size ); trim_(); return path_; }
        const xpath& trim_( const char* name ) { if( *name ) { path_ = path_.head(); } return path_; }
        void trim_() { path_ = path_.head(); }
        static std::string value_( const boost::posix_time::ptime& t ) { return boost::posix_time::to_iso_string( t ); }
        template < typename T > static T value_( T v ) { return v; }
};

} // namespace comma

#endif /*COMMA_NAME_VALUE_PTREE_H_*/
