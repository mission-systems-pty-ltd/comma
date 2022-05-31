// Copyright (c) 2011 The University of Sydney

/// @author cedric wohlleber
/// @author vsevolod vlaskine

#include <iostream>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/regex.hpp>
#include <boost/unordered_set.hpp>
#include "../base/exception.h"
#include "../base/types.h"
#include "../string/string.h"
#include "../xpath/xpath.h"
#include "../visiting/visit.h"
#include "../visiting/while.h"
#include "ptree.h"

namespace comma {

void property_tree::put( boost::property_tree::ptree& ptree, const xpath& path, const std::string& value, bool use_index )
{
    boost::property_tree::ptree* t = &ptree;
    for( unsigned int i = 0; i < path.elements.size(); ++i )
    {
        boost::optional< boost::property_tree::ptree& > child = t->get_child_optional( path.elements[i].name );
        if( path.elements[i].index )
        {
            std::string name = use_index ? "" : path.elements[i].name;
            unsigned int size = 0;
            if( child ) // quick and dirty, because some parts of boost::property_tree suck
            {
                if( use_index ) { t = &(*child); }
                bool found = false;
                for( boost::property_tree::ptree::assoc_iterator j = t->ordered_begin(); j != t->not_found(); ++j )
                {
                    if( j->first != name ) { if( found ) { break; } else { continue; } }
                    ++size;
                    if( *path.elements[i].index < size ) { t = &( j->second ); break; }
                    found = true;
                }
            }
            else if( use_index ) { t = &( t->add_child( path.elements[i].name, boost::property_tree::ptree() ) ); }
            if( *path.elements[i].index > size ) { COMMA_THROW( comma::exception, "expected index not greater than " << size << "; got " << path.elements[i].index << " in " << path.to_string() ); }
            if( *path.elements[i].index == size ) { t = &( name.empty() ? t->push_back( std::make_pair( name, boost::property_tree::ptree() ) )->second : t->add_child( name, boost::property_tree::ptree() ) ); }
        }
        else
        {
            t = child ? &( *child ) : &t->add_child( path.elements[i].name, boost::property_tree::ptree() );
        }
    }
    t->put_value( value );
}

boost::optional< const boost::property_tree::ptree& > property_tree::get_tree( const boost::property_tree::ptree& ptree, const xpath& path, bool use_index )
{
    const boost::property_tree::ptree* t = &ptree;
    for( unsigned int i = 0; i < path.elements.size(); ++i )
    {
        boost::optional< const boost::property_tree::ptree& > child = t->get_child_optional( path.elements[i].name );
        if( path.elements[i].index )
        {
            std::string name = use_index ? "" : path.elements[i].name;
            unsigned int size = 0;
            if( child ) // quick and dirty, because some parts of boost::property_tree suck
            {
                if(use_index)
                    t = &( *child );
                bool found = false;
                for( boost::property_tree::ptree::const_assoc_iterator j = t->ordered_begin(); j != t->not_found(); ++j )
                {
                    if( j->first != name ) { if( found ) { break; } else { continue; } }
                    ++size;
                    if( *path.elements[i].index < size ) { t = &( j->second ); break; }
                    found = true;
                }
            }
            if( *path.elements[i].index >= size ) { return boost::none; }
        }
        else
        {
            if( !child ) { return boost::none; }
            t = &( *child );
        }
    }
    return *t;
}

boost::optional< boost::property_tree::ptree& > property_tree::get_tree( boost::property_tree::ptree& ptree, const xpath& path, bool use_index )
{
    boost::optional< const boost::property_tree::ptree& > t = get_tree( const_cast< const boost::property_tree::ptree& >( ptree ), path, use_index );
    if( !t ) { return boost::none; }
    return const_cast< boost::property_tree::ptree& >( *t );
}

boost::optional< std::string > property_tree::get( const boost::property_tree::ptree& ptree, const xpath& path, bool use_index )
{
    boost::optional< const boost::property_tree::ptree& > t = get_tree( ptree, path, use_index );
    if( !t ) { return boost::none; }
    return t->get_value_optional< std::string >();
}

} // namespace comma {

namespace comma { namespace impl {

static void ptree_output_value_( std::ostream& os, const std::string& value, bool is_begin, const xpath& path, char equal_sign, char delimiter, const std::string& root, bool const unquote_numbers )
{
    static boost::regex number_like_string( "^0[0-9][0-9]*$" );
    if( !is_begin ) { os << delimiter; }
    if( root != "" ) { os << root << "/"; }
    os << path.to_string() << equal_sign;
    bool quoted = true;
    if( unquote_numbers )
    {
        if( "true" == value || "false" == value ) { quoted = false; }
        else if( !boost::regex_match( value, number_like_string ) ) { try { boost::lexical_cast< double >( value ); quoted = false; } catch ( ... ) {} }
    }
    if( quoted ) { os << '"' << value << '"'; } else { os << value; }
}

static void ptree_to_path_value_string_impl( std::ostream& os
                                           , boost::property_tree::ptree::const_iterator i
                                           , bool is_begin
                                           , xpath& path
                                           , xpath& display_path
                                           , property_tree::path_mode mode
                                           , char equal_sign
                                           , char delimiter
                                           , const std::string& root
                                           , bool const unquote_numbers )
{
    if( i->second.begin() == i->second.end() )
    {
        ptree_output_value_( os, i->second.get_value< std::string >(), is_begin, display_path / i->first, equal_sign, delimiter, root, unquote_numbers );
    }
    else
    {
        path /= i->first;
        display_path /= i->first;
        boost::optional< std::string > v = i->second.get_value_optional< std::string >();
        if( v ) // quick and dirty
        {
            const std::string& stripped = comma::strip( *v );
            if( !stripped.empty() )  { ptree_output_value_( os, stripped, is_begin, display_path, equal_sign, delimiter, root, unquote_numbers );  }
        }

        comma::uint32 index=0;
        for( boost::property_tree::ptree::const_iterator j = i->second.begin(); j != i->second.end(); ++j )
        {
            // Test if it is json array data, if so all keys are empty. If so display indices in path if requested
            if( mode == property_tree::without_brackets && j->first.empty()  ) { display_path /= boost::lexical_cast< std::string >( index++ ); }
            else if( mode == property_tree::with_brackets && j->first.empty() ) { display_path.elements.back().index = index++; }
            ptree_to_path_value_string_impl( os, j, is_begin, path, display_path, mode, equal_sign, delimiter, root, unquote_numbers );
            if( mode == property_tree::without_brackets && j->first.empty() ) { display_path = display_path.head(); }
            is_begin = false;
        }
        if( !(i->first.empty()) ) { path = path.head(); display_path = display_path.head(); } // for json arrays, the keys are empty
    }
}

} } // namespace { namespace impl {


namespace comma {

void property_tree::to_path_value( std::ostream& os, const boost::property_tree::ptree& ptree, path_mode mode, char equal_sign, char delimiter, const xpath& root, bool const unquote_numbers )
{
    for( boost::property_tree::ptree::const_iterator i = ptree.begin(); i != ptree.end(); ++i )
    {
        // display_path is the modified key path showing array indices, if array exists within e.g abc[0]/xyz[0]
        // But the actual path to the value is many empty keys under abc and abc/xyz
        // Boost: "JSON arrays are mapped to nodes. Each element is a child node with an empty name.
        //         If a node has both named and unnamed child nodes, it cannot be mapped to a JSON representation."
        // http://www.boost.org/doc/libs/1_41_0/doc/html/boost_propertytree/parsers.html#boost_propertytree.parsers.json_parser
        xpath path;
        xpath display_path;
        impl::ptree_to_path_value_string_impl( os, i, i == ptree.begin(), path, display_path, mode, equal_sign, delimiter, root.to_string(), unquote_numbers ); // quick and dirty
    }
}

void property_tree::from_path_value( std::istream& is, boost::property_tree::ptree& ptree, property_tree::path_value::check_repeated_paths check_type, char equal_sign, char delimiter, bool use_index )
{
    std::string s;
    while( is.good() && !is.eof() ) // quick and dirty: read to the end of file
    {
        std::string line;
        std::getline( is, line );
        std::string::size_type pos = line.find_first_not_of( ' ' );
        if( pos == std::string::npos || line[pos] == '#' ) { continue; }
        s += line + delimiter;
    }
    ptree = comma::property_tree::from_path_value_string( s, equal_sign, delimiter, check_type, use_index );
}

std::string property_tree::to_path_value_string( const boost::property_tree::ptree& ptree, property_tree::path_mode mode, char equal_sign, char delimiter, bool const unquote_numbers )
{
    std::ostringstream oss;
    to_path_value( oss, ptree, mode, equal_sign, delimiter, xpath(), unquote_numbers );
    return oss.str();
}

namespace impl {

template< property_tree::path_value::check_repeated_paths check_type > struct path_filter;

template <> struct path_filter< property_tree::path_value::no_check >
{
    path_filter( boost::property_tree::ptree& ptree ) {}
    bool put_allowed( const std::string&, bool ) const { return true; }
};

template <> struct path_filter< property_tree::path_value::no_overwrite >
{
    path_filter( boost::property_tree::ptree& ptree ) : ptree_( ptree ) {}
    bool put_allowed( const std::string& p, bool use_index ) const
    {
        if( use_index )
        {
            if( property_tree::get( ptree_, p, use_index ) ) { COMMA_THROW( comma::exception, "input path '" << p << "' already in the tree" ); }
        }
        else
        {
            boost::optional< std::string > old_v = ptree_.get_optional< std::string >( boost::property_tree::ptree::path_type( p, '/' ) );
            if( old_v ) { COMMA_THROW( comma::exception, "input path '" << p << "' already in the tree" ); }
        }
        return true;
    }
    boost::property_tree::ptree& ptree_;
};

template <> struct path_filter< property_tree::path_value::take_last >
{
    path_filter( boost::property_tree::ptree& ptree ) {}
    bool put_allowed( const std::string&, bool ) const { return true; }
};

template <> struct path_filter< property_tree::path_value::unique_input >
{
    path_filter( boost::property_tree::ptree& ptree ) : ptree_( ptree ) {}
    bool put_allowed( const std::string& p, bool use_index ) const
    {
        typedef std::pair< path_set::iterator, bool > result_of_insert;
        result_of_insert result = unique_.insert( p );
        if( !result.second ) { COMMA_THROW( comma::exception, "input path '" << p << "' is not unique" ); }
        return true;
    }
    boost::property_tree::ptree& ptree_;
private:
    typedef boost::unordered_set< std::string > path_set;
    mutable path_set unique_;
};

template < property_tree::path_value::check_repeated_paths check_type > struct from_path_value_string
{
    static inline boost::property_tree::ptree& parse( boost::property_tree::ptree& ptree
                                                    , const std::string& s
                                                    , char equal_sign
                                                    , char delimiter
                                                    , bool use_index ) // todo? make using index default?
    {
        const std::vector< std::string >& v = comma::split( s, delimiter );
        impl::path_filter< check_type > c( ptree );

        for( std::size_t i = 0; i < v.size(); ++i )
        {
            if( v[i].empty() ) { continue; }
            std::string::size_type p = v[i].find_first_of( equal_sign );
            if( p == std::string::npos ) { COMMA_THROW( comma::exception, "expected '" << delimiter << "'-separated xpath" << equal_sign << "value pairs; got \"" << v[i] << "\"" ); }
            const std::string& path = comma::strip( v[i].substr( 0, p ), '"' );
            const std::string& value = comma::strip( v[i].substr( p + 1, std::string::npos ), '"' );
            if( c.put_allowed( path, use_index ) )
            {
                if( use_index ) { property_tree::put( ptree, path, value, use_index ); } // quick and dirty
                else { ptree.put( boost::property_tree::ptree::path_type( path, '/' ), value ); }
            }
        }
        return ptree;
    }
};

} // namespace impl {

boost::property_tree::ptree& property_tree::from_path_value_string( boost::property_tree::ptree& ptree, const std::string& s, char equal_sign, char delimiter, property_tree::path_value::check_repeated_paths check_type, bool use_index )
{
    switch ( check_type ) {
        case comma::property_tree::path_value::take_last         : return impl::from_path_value_string< comma::property_tree::path_value::take_last >::parse( ptree, s, equal_sign, delimiter, use_index );
        case comma::property_tree::path_value::unique_input      : return impl::from_path_value_string< comma::property_tree::path_value::unique_input >::parse( ptree, s, equal_sign, delimiter, use_index );
        case comma::property_tree::path_value::no_overwrite      : return impl::from_path_value_string< comma::property_tree::path_value::no_overwrite >::parse( ptree, s, equal_sign, delimiter, use_index );
        case comma::property_tree::path_value::no_check: default : return impl::from_path_value_string< comma::property_tree::path_value::no_check >::parse( ptree, s, equal_sign, delimiter, use_index );
    }
    return ptree;
}

boost::property_tree::ptree property_tree::from_path_value_string( const std::string& s, char equal_sign, char delimiter, property_tree::path_value::check_repeated_paths check_type, bool use_index )
{
    boost::property_tree::ptree ptree;
    from_path_value_string( ptree, s, equal_sign, delimiter, check_type, use_index );
    return ptree;
}

bool is_seekable( std::istream& stream ) { return static_cast< bool >( stream.seekg( 0, std::ios::beg ) ); }

void property_tree::from_unknown( std::istream& stream, boost::property_tree::ptree& ptree, property_tree::path_value::check_repeated_paths check_type, char equal_sign, char delimiter, bool use_index )
{
    if( is_seekable( stream ) )
    {
        from_unknown_seekable( stream, ptree, check_type, equal_sign, delimiter, use_index );
    }
    else
    {
        std::stringstream buffer;
        buffer << stream.rdbuf();
        from_unknown_seekable( buffer, ptree, check_type, equal_sign, delimiter, use_index );
    }
}

static boost::property_tree::ptree ptree_to_xml_(const boost::property_tree::ptree& ptree)
{
    boost::property_tree::ptree out= boost::property_tree::ptree();
    //copy all children
    for (boost::property_tree::ptree::const_assoc_iterator i=ptree.ordered_begin(); i != ptree.not_found(); i++ )
    {
        if(i->second.find("") != i->second.not_found())
        {
            //colapse array
            for (boost::property_tree::ptree::const_assoc_iterator j=i->second.ordered_begin(); j != i->second.not_found(); j++ )
            {
                out.add_child(i->first, ptree_to_xml_(j->second));
            }
        }
        else
            out.add_child(i->first, ptree_to_xml_(i->second));
    }
    out.put_value(ptree.get_value<std::string>());
    return out;
}

static boost::property_tree::ptree xml_to_ptree_( boost::property_tree::ptree& ptree)
{
    boost::property_tree::ptree out= boost::property_tree::ptree();
    boost::property_tree::ptree unnamed_array= boost::property_tree::ptree();
    for ( boost::property_tree::ptree::iterator i=ptree.begin(); i!=ptree.end(); i++ )
    {
        //look ahead for duplicate name
        boost::property_tree::ptree::iterator lah = i;
        if ( ++lah != ptree.end() && i->first == lah->first )
        {
            //add to unnamed array
            unnamed_array.push_back( std::make_pair( "", xml_to_ptree_( i->second ) ) );
        }
        else
        {
            if(unnamed_array.size()!=0)
            {
                //assert((i-1)->first==i->first);
                //the last of duplicated name
                unnamed_array.push_back( std::make_pair( "", xml_to_ptree_( i->second ) ) );
                out.add_child(i->first,unnamed_array);
                unnamed_array= boost::property_tree::ptree();
            }
            else
            {
                out.add_child(i->first, xml_to_ptree_(i->second) );
            }
        }
    }
    out.put_value( comma::strip( ptree.get_value<std::string>(), " \n\t" ));
    return out;
}

void property_tree::read_xml( std::istream& is, boost::property_tree::ptree& ptree )
{
        boost::property_tree::read_xml( is, ptree );
        ptree=xml_to_ptree_(ptree);
}

void property_tree::write_xml( std::ostream& os, const boost::property_tree::ptree& ptree, const xml_writer_settings_t& xml_writer_settings)
{
        boost::property_tree::write_xml( os, ptree_to_xml_(ptree), xml_writer_settings );
}

void property_tree::from_unknown_seekable( std::istream& stream, boost::property_tree::ptree& ptree, property_tree::path_value::check_repeated_paths check_type, char equal_sign, char delimiter, bool use_index )
{
    if( !is_seekable( stream ) ) { COMMA_THROW( comma::exception, "input stream is not seekable" ); }
    try
    {
        stream.clear();
        stream.seekg( 0, std::ios::beg );
        boost::property_tree::read_json( stream, ptree );
        return;
    }
    catch( const boost::property_tree::ptree_error&  ex ) {}
    catch(...) { throw; }
    try
    {
        stream.clear();
        stream.seekg( 0, std::ios::beg );
        comma::property_tree::read_xml( stream, ptree );
        return;
    }
    catch( const boost::property_tree::ptree_error&  ex ) {}
    catch(...) { throw; }
    try
    {
        stream.clear();
        stream.seekg( 0, std::ios::beg );
        comma::property_tree::from_path_value( stream, ptree, check_type, equal_sign, delimiter, use_index );
        return;
    }
    catch( const boost::property_tree::ptree_error&  ex ) {}
    catch( const comma::exception&  ex ) {}
    catch(...) { throw; }
    // TODO: add try for ini format (currently the problem is that path-value treats ini sections and comments as valid entries; possible solution: make path-value parser stricter)
    COMMA_THROW( comma::exception, "failed to guess format" );
}

} // namespace comma {
