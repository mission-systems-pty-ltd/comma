// Copyright (c) 2011 The University of Sydney
// Copyright (c) 2022 Vsevolod Vlaskine

/// @authors cedric wohlleber, vsevolod vlaskine

#include "../base/exception.h"
#include "map.h"

namespace comma { namespace name_value {

map::map( const std::string& line, char delimiter, char value_delimiter, bool unique, const std::string& allowed_names ): _line( line ) { init_( impl::options( delimiter, value_delimiter ), unique, allowed_names ); }

map::map( const std::string& line, const std::string& fields, char delimiter, char value_delimiter, bool unique, const std::string& allowed_names ): _line( line ) { init_( impl::options( fields, delimiter, value_delimiter ), unique, allowed_names ); }
map::map( const std::string& line, const comma::name_value::impl::options& options, bool unique, const std::string& allowed_names ): _line( line ) { init_( options, unique, allowed_names ); }

static std::vector< std::string > get_named_values( const std::string& line, const comma::name_value::impl::options& options )
{
    std::vector< std::string > named_values = split_escaped( line, options.m_delimiter, &(options.m_quotes[0]), options.m_escape );
    for( std::size_t i = 0; i < options.m_names.size() && i < named_values.size(); ++i )
    {
        if( options.m_names[i].empty() ) { continue; }
        if( split( named_values[i], options.m_value_delimiter ).size() != 1U ) { COMMA_THROW( comma::exception, "expected unnamed value for " << options.m_names[i] << ", got: " << named_values[i] ); }
        named_values[i] = options.m_names[i] + options.m_value_delimiter + named_values[i];
    }
    return named_values;
}

void map::init_( const comma::name_value::impl::options& options, bool unique, const std::string& allowed_names )
{
    std::unordered_set< std::string > allowed;
    for( auto name: comma::split( allowed_names, ',', true ) ) { allowed.insert( name ); }
    const std::vector< std::string >& named_values = get_named_values( _line, options );
    for( std::size_t i = 0; i < named_values.size(); ++i )
    {
        std::vector< std::string > pair = split_escaped( named_values[i], options.m_value_delimiter, &( options.m_quotes[0]), options.m_escape );
        if( !allowed.empty() && allowed.find( pair[0] ) == allowed.end() ) { COMMA_THROW( comma::exception, "name \"" << pair[0] << "\" is not among allowed names: " << allowed_names ); }
        if( unique && pair.size() > 0 && _map.find( pair[0] ) != _map.end() ) { COMMA_THROW( comma::exception, "expected unique names, got more than one \"" << pair[0] << "\"" ); }
        switch( pair.size() )
        {
            case 1: _map.insert( std::make_pair( pair[0], std::string() ) ); break; // quick and dirty
            case 2: _map.insert( std::make_pair( pair[0], pair[1] ) ); break;
            default: { COMMA_THROW( comma::exception, "expected name-value pair, got: " << join( pair, options.m_value_delimiter ) ); }
        }
    }
}

std::vector< std::pair< std::string, std::string > > map::as_vector( const std::string& line, char delimiter, char value_delimiter ) { return as_vector( line, impl::options( delimiter, value_delimiter ) ); }

std::vector< std::pair< std::string, std::string > > map::as_vector( const std::string& line, const std::string& fields, char delimiter, char value_delimiter ) { return as_vector( line, impl::options( fields, delimiter, value_delimiter ) ); }

std::vector< std::pair< std::string, std::string > > map::as_vector( const std::string& line, const impl::options& options )
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

bool map::exists( const std::string& name ) const { return _map.find( name ) != _map.end(); }

void map::assert_mutually_exclusive( const std::string& f ) { assert_mutually_exclusive( comma::split( f, ',' ) ); }

void map::assert_mutually_exclusive( const std::string& f, const std::string& g ) { assert_mutually_exclusive( comma::split( f, ',' ), comma::split( g, ',' ) ); }

void map::assert_mutually_exclusive( const std::vector< std::string >& f )
{
    std::string found;
    for( const auto& s: f )
    {
        if( _map.find( s ) == _map.end() ) { continue; }
        if( !found.empty() ) { COMMA_THROW( comma::exception, found << " and " << s << " are mutually exclusive" ); }
        found = s;
    }
}

void map::assert_mutually_exclusive( const std::vector< std::string >& f, const std::vector< std::string >& g )
{
    std::string found;
    for( const auto& s: f ) { if( _map.find( s ) != _map.end() ) { found = s; break; } }
    if( found.empty() ) { return; }
    for( const auto& s: g )
    {
        if( _map.find( s ) != _map.end() ) { COMMA_THROW( comma::exception, found << " and " << s << " are mutually exclusive" ); }
    }    
}

} } // namespace comma { namespace name_value {
