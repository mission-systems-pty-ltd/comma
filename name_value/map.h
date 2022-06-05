// Copyright (c) 2011 The University of Sydney

/// @authors cedric wohlleber, vsevolod vlaskine

#pragma once

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_parsers.hpp>
#include <boost/optional.hpp>
#include <boost/lexical_cast.hpp>
#include "impl/options.h"

namespace comma { namespace name_value {

/// constructs a map of name-value pair from an input string
/// TODO implement full_path_as_name ?
class map
{
    public:
        /// constructor
        map( const std::string& line, char delimiter = ';', char value_delimiter = '=', bool unique = false );
        /// constructor
        map( const std::string& line, const std::string& fields, char delimiter = ';', char value_delimiter = '=', bool unique = false );
        /// constructor
        map( const std::string& line, const impl::options& options, bool unique = false );

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

    private:
        void init_( const comma::name_value::impl::options& options, bool unique );
        const std::string _line;
        map_type _map;
};

inline map::map( const std::string& line, char delimiter, char value_delimiter, bool unique ): _line( line ) { init_( impl::options( delimiter, value_delimiter ), unique ); }

inline map::map( const std::string& line, const std::string& fields, char delimiter, char value_delimiter, bool unique ): _line( line ) { init_( impl::options( fields, delimiter, value_delimiter ), unique ); }

inline map::map( const std::string& line, const comma::name_value::impl::options& options, bool unique ): _line( line ) { init_( options, unique ); }

inline static std::vector< std::string > get_named_values( const std::string& line, const comma::name_value::impl::options& options )
{
    std::vector< std::string > named_values = split_escaped( line, options.m_delimiter, &(options.m_quotes[0]), options.m_escape );
    for( std::size_t i = 0; i < options.m_names.size() && i < named_values.size(); ++i )
    {
        if( options.m_names[i].empty() ) { continue; }
        if( split( named_values[i], options.m_value_delimiter ).size() != 1U ) { COMMA_THROW_STREAM( comma::exception, "expected unnamed value for " << options.m_names[i] << ", got: " << named_values[i] ); }
        named_values[i] = options.m_names[i] + options.m_value_delimiter + named_values[i];
    }
    return named_values;
}

inline void map::init_( const comma::name_value::impl::options& options, bool unique )
{
    const std::vector< std::string >& named_values = get_named_values( _line, options );
    for( std::size_t i = 0; i < named_values.size(); ++i )
    {
        std::vector< std::string > pair = split_escaped( named_values[i], options.m_value_delimiter, &( options.m_quotes[0]), options.m_escape );
        if( unique && pair.size() > 0 && _map.find( pair[0] ) != _map.end() ) { COMMA_THROW_STREAM( comma::exception, "expected unique names, got more than one \"" << pair[0] << "\"" ); }
        switch( pair.size() )
        {
            case 1: _map.insert( std::make_pair( pair[0], std::string() ) ); break; // quick and dirty
            case 2: _map.insert( std::make_pair( pair[0], pair[1] ) ); break;
            default: { COMMA_THROW_STREAM( comma::exception, "expected name-value pair, got: " << join( pair, options.m_value_delimiter ) ); }
        }
    }
}

inline std::vector< std::pair< std::string, std::string > > map::as_vector( const std::string& line, char delimiter, char value_delimiter ) { return as_vector( line, impl::options( delimiter, value_delimiter ) ); }

inline std::vector< std::pair< std::string, std::string > > map::as_vector( const std::string& line, const std::string& fields, char delimiter, char value_delimiter ) { return as_vector( line, impl::options( fields, delimiter, value_delimiter ) ); }

inline std::vector< std::pair< std::string, std::string > > map::as_vector( const std::string& line, const impl::options& options )
{
    std::vector< std::pair< std::string, std::string > > v;
    const std::vector< std::string >& named_values = get_named_values( line, options );
    for( std::size_t i = 0; i < named_values.size(); ++i )
    {
        std::vector< std::string > pair = split_escaped( named_values[i], options.m_value_delimiter, &(options.m_quotes[0]), options.m_escape );
        switch( pair.size() )
        {
            case 1: v.push_back( std::make_pair( pair[0], std::string() ) ); break; // quick and dirty
            case 2: v.push_back( std::make_pair( pair[0], pair[1] ) ); break;
            default: { COMMA_THROW_STREAM( comma::exception, "expected name-value pair, got: " << join( pair, options.m_value_delimiter ) ); }
        }
    }
    return v;
}

inline bool map::exists( const std::string& name ) const { return _map.find( name ) != _map.end(); }

namespace detail {

template < typename T >
inline T lexical_cast( const std::string& s ) { return boost::lexical_cast< T >( s ); }

template <>
inline bool lexical_cast< bool >( const std::string& s )
{
    if( s == "" || s == "true" ) { return true; }
    if( s == "false" ) { return false; }
    return boost::lexical_cast< bool >( s );
}

template <>
inline boost::posix_time::ptime lexical_cast< boost::posix_time::ptime >( const std::string& s )
{
    if ( s == "not-a-date-time" ) { return boost::posix_time::not_a_date_time; }
    else if ( s == "+infinity" || s == "+inf" || s == "inf" ) { return boost::posix_time::pos_infin; }
    else if ( s == "-infinity" || s == "-inf" ) { return boost::posix_time::neg_infin; }
    else return boost::posix_time::from_iso_string( s );
}

} // namespace detail {

template < typename T >
inline std::vector< T > map::values( const std::string& name ) const
{
    std::vector< T > v;
    for( typename map_type::const_iterator it = _map.begin(); it != _map.end(); ++it )
    {
        if( it->first == name ) { v.push_back( detail::lexical_cast< T >( it->second ) ); }
    }
    return v;
}

template < typename T >
inline T map::value( const std::string& name, const T& default_value ) const
{
    const std::vector< T >& v = values< T >( name );
    return v.empty() ? default_value : v[0];
}

template < typename T >
inline T map::value( const std::string& name ) const
{
    const std::vector< T >& v = values< T >( name );
    if( v.empty() ) { COMMA_THROW_STREAM( comma::exception, "'" << name << "' not found in \"" << _line << "\"" ); }
    return v[0];
}

template < typename T >
inline boost::optional< T > map::optional( const std::string& name ) const
{
    const std::vector< T >& v = values< T >( name );
    return v.empty() ? boost::optional< T >() : boost::optional< T >( v[0] );
}

} } // namespace comma { namespace name_value {
