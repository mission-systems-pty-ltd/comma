// Copyright (c) 2011 The University of Sydney

/// @authors cedric wohlleber, vsevolod vlaskine

#pragma once

#include <unordered_set>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_parsers.hpp>
#include <boost/optional.hpp>
#include <boost/lexical_cast.hpp>
#include "../string/string.h"
#include "impl/options.h"

namespace comma { namespace name_value {

/// constructs a map of name-value pair from an input string
/// TODO implement full_path_as_name ?
class map
{
    public:
        /// constructor
        map( const std::string& line, char delimiter = ';', char value_delimiter = '=', bool unique = false, const std::string& allowed_names = "" );
        /// constructor
        map( const std::string& line, const std::string& fields, char delimiter = ';', char value_delimiter = '=', bool unique = false, const std::string& allowed_names = "" );
        /// constructor
        map( const std::string& line, const impl::options& options, bool unique = false, const std::string& allowed_names = "" );

        /// return vector of name-value pairs in the given order
        static std::vector< std::pair< std::string, std::string > > as_vector( const std::string& line, char delimiter = ';', char value_delimiter = '=' );

        /// return vector of name-value pairs in the given order
        static std::vector< std::pair< std::string, std::string > > as_vector( const std::string& line, const std::string& fields, char delimiter = ';', char value_delimiter = '=' );

        /// return vector of name-value pairs in the given order
        static std::vector< std::pair< std::string, std::string > > as_vector( const std::string& line, const impl::options& options );

        /// return true, if field exists
        bool exists( const std::string& name ) const;

        /// return values for all the fields with a given name
        template < typename T >
        std::vector< T > values( const std::string& name ) const;

        /// return first available value, if field exists; otherwise return default
        template < typename T >
        T value( const std::string& name, const T& default_value ) const;

        /// return first available value, if field exists; otherwise throw
        template < typename T >
        T value( const std::string& name ) const;

        /// return first available value, if field exists; otherwise return empty optional
        template < typename T >
        boost::optional< T > optional( const std::string& name ) const;

        /// map type
        typedef std::multimap< std::string, std::string > map_type;

        /// return name-value map
        const map_type& get() const { return _map; }

        /// throw exception if incompatible fields are present
        void assert_mutually_exclusive( const std::string& f );
        void assert_mutually_exclusive( const std::string& f, const std::string& g );
        void assert_mutually_exclusive( const std::vector< std::string >& f );
        void assert_mutually_exclusive( const std::vector< std::string >& f, const std::vector< std::string >& g );

    private:
        void init_( const comma::name_value::impl::options& options, bool unique, const std::string& allowed_names );
        const std::string _line;
        map_type _map;
};

namespace detail {

template < typename T > inline T lexical_cast( const std::string& s ) { return boost::lexical_cast< T >( s ); }

template <> inline bool lexical_cast< bool >( const std::string& s )
{
    if( s == "" || s == "true" ) { return true; }
    if( s == "false" ) { return false; }
    return boost::lexical_cast< bool >( s );
}

template <> inline boost::posix_time::ptime lexical_cast< boost::posix_time::ptime >( const std::string& s )
{
    if ( s == "not-a-date-time" ) { return boost::posix_time::not_a_date_time; }
    else if ( s == "+infinity" || s == "+inf" || s == "inf" ) { return boost::posix_time::pos_infin; }
    else if ( s == "-infinity" || s == "-inf" ) { return boost::posix_time::neg_infin; }
    else return boost::posix_time::from_iso_string( s );
}

} // namespace detail {

template < typename T > inline std::vector< T > map::values( const std::string& name ) const
{
    std::vector< T > v;
    for( typename map_type::const_iterator it = _map.begin(); it != _map.end(); ++it )
    {
        if( it->first == name ) { v.push_back( detail::lexical_cast< T >( it->second ) ); }
    }
    return v;
}

template < typename T > inline T map::value( const std::string& name, const T& default_value ) const
{
    const std::vector< T >& v = values< T >( name );
    return v.empty() ? default_value : v[0];
}

template < typename T > inline T map::value( const std::string& name ) const
{
    const std::vector< T >& v = values< T >( name );
    if( v.empty() ) { COMMA_THROW_STREAM( comma::exception, "'" << name << "' not found in \"" << _line << "\"" ); }
    return v[0];
}

template < typename T > inline boost::optional< T > map::optional( const std::string& name ) const
{
    const std::vector< T >& v = values< T >( name );
    return v.empty() ? boost::optional< T >() : boost::optional< T >( v[0] );
}

} } // namespace comma { namespace name_value {
