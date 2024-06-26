// Copyright (c) 2011 The University of Sydney

/// @author cedric wohlleber

#pragma once

#include "../visiting/apply.h"
#include "../name_value/map.h"
#include "../name_value/impl/options.h"
#include "../name_value/impl/from_name_value.h"
#include "../name_value/impl/to_name_value.h"

namespace comma { namespace name_value {
    
/// parser for semicolon-separated name-value string
class parser
{
public:
    /// constructor
    /// @param delimiter delimiter between named values
    /// @param value_delimiter delimiter between name and value
    /// @param full_path_as_name use full path as name
    parser( char delimiter = ';', char value_delimiter = '=', bool full_path_as_name = true );
    
    /// constructor
    /// @param delimiter delimiter between named values
    /// @param value_delimiter delimiter between name and value
    /// @param full_path_as_name use full path as name
    /// @param fields default names for unnamed value
    parser( const std::string& fields, char delimiter = ';', char value_delimiter = '=', bool full_path_as_name = true );

    /// get struct from string
    template < typename S >
    S get( const std::string& line, const S& default_s = S() ) const;
    
    /// put struct into string
    template < typename S >
    std::string put( const S& s ) const;
    
    /// put struct into string
    /// @todo implement
    template < typename S >
    void put( std::string& line, const S& s ) const;

    /// mangle string as in following examples
    ///     - prefix: "abc"; line: "x=1;y/z=2"; mangled: "abc/x=1;abc/y/z=2"
    ///     - self-mangled (prefix: ""); line: "my-operation;x=1;y/z=2"; mangled: "my-operation/x=1;my-operation/y/z=2"
    /// usage example
    ///     struct naming { static std::array< std::string, 3 > names() { return { "some-operation", "another-operation" } } };
    ///     struct some_operation { int a; float b; };
    ///     struct another_operation { double c; std::string d; };
    ///     typedef comma::named_variant< naming, some_operation, another_operation > operation_t;
    ///     operation_t operation = parser().get< operation_t >( parser::mangled( operation_options ) );
    ///     if( operation.is< some_operation >() ) { /* handle */ }
    ///     else if( operation.is< some_operation >() ) { /* handle */ }
    ///     etc
    static std::string mangled( const std::string& line, const std::string& prefix = "", char delimiter = ';' );

private:
    impl::options _options;
};

inline parser::parser( char delimiter, char value_delimiter, bool full_path_as_name ): _options( delimiter, value_delimiter, full_path_as_name ) {}

inline parser::parser( const std::string& fields, char delimiter, char value_delimiter, bool full_path_as_name ): _options( fields, delimiter, value_delimiter, full_path_as_name ) {}

template < typename S >
inline S parser::get( const std::string& line, const S& default_s ) const
{
    map::map_type m = map( line, _options ).get();
    name_value::impl::from_name_value from_name_value( m, _options.m_full_path_as_name );
    S s = default_s;
    visiting::apply( from_name_value ).to( s );
    return s;
}

template < typename S >
inline std::string parser::put( const S& s ) const
{
    name_value::impl::to_name_value toname_value( _options.m_value_delimiter, _options.m_full_path_as_name );
    visiting::apply( toname_value ).to( s );
    return join( toname_value.strings(), _options.m_delimiter );
}

} } // namespace comma { namespace name_value {
