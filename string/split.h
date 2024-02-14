// Copyright (c) 2011 The University of Sydney

/// @author vsevolod vlaskine

#pragma once

#include <array>
#include <string>
#include <vector>
#include <boost/array.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>

namespace comma {

namespace string {

bool is_one_of( char c, const char* characters );

} // namespace string {

/// split string into tokens (a quick implementation); always contains at least one element unless empty_if_empty_input is true
std::vector< std::string > split( const std::string& s, const char* separators = " ", bool empty_if_empty_input = false );

/// split string into tokens (a quick implementation); always contains at least one element unless empty_if_empty_input is true
std::vector< std::string > split( const std::string& s, char separator, bool empty_if_empty_input = false );

/// split string into tokens and cast to a vector of given types
template < typename T > std::vector< T > split_as( const std::string& s, const char* separators );
template < typename T > std::vector< T > split_as( const std::string& s, const char* separators, const T& default_value );
template < typename T > std::vector< T > split_as( const std::string& s, char separator );
template < typename T > std::vector< T > split_as( const std::string& s, char separator, const T& default_value );
template < typename T > std::vector< T > split_as( const std::string& s, const char* separators, const std::vector< T >& defaults ); // todo: re-implement using traits
template < typename T > std::vector< T > split_as( const std::string& s, char separator, const std::vector< T >& defaults );
template < typename T, std::size_t N > std::vector< T > split_as( const std::string& s, const char* separators, const std::array< T, N >& defaults );
template < typename T, std::size_t N > std::vector< T > split_as( const std::string& s, char separator, const std::array< T, N >& defaults );
template < typename T, std::size_t N > std::vector< T > split_as( const std::string& s, const char* separators, const boost::array< T, N >& defaults );
template < typename T, std::size_t N > std::vector< T > split_as( const std::string& s, char separator, const boost::array< T, N >& defaults );

/// Split string into tokens; always contains at least one element;
/// skips backslash escaped separator, handle non-nested quotes;
/// exceptions thrown on errors.
///
/// An escape character can be anywhere in the string.
/// An escape character at end of string.will be kept.
/// An escape character will only escape a delimiter, quote or escape character;
/// escaping any other character will result in both being kept;
/// e.g. c:\windows\ will be kept as c:\windows\ with the trailing backslash
/// e.g. filename;delimiter=\\;fields=a,b will be kept as filename;delimiter=\;fields=a,b
///
/// A quote may be anywhere in a string. Quotes must be closed; i.e Each start
/// quote must be paired with an end quote, or an exception is thrown.
/// Quotes don't nest and can not be mixed; e.g. a ' will not close a " quoted string.
/// However "'" and '"' are perfectly legal strings of ' and "
std::vector< std::string > split_escaped( const std::string& s, const char * separators = " ", const char * quotes = "\"\'", char escape = '\\' );
/// split string into tokens; always contains at least one element;
/// skips backslash escaped separator, handle boolean quotes
std::vector< std::string > split_escaped( const std::string& s, char separator, const char * quotes = "\"\'", char escape = '\\' );
/// skips bracketed separators
std::vector< std::string > split_bracketed( const std::string& s, const char * separators = " ", char lbracket = '(', char rbrackets = ')', bool strip_brackets = true );
/// skips bracketed separators
std::vector< std::string > split_bracketed( const std::string& s, char separator, char lbracket = '(', char rbracket = ')', bool strip_brackets = true );

namespace impl {

template < typename T >
inline std::vector< T > split_with_scalar_default( const std::string& s, const char* separators, const boost::optional< T >& default_value )
{
    const auto& v = split( s, separators, true );
    std::vector< T > t( v.size() );
    for( unsigned int i = 0; i < v.size(); ++i ) { t[i] = v[i].empty() && default_value ? *default_value : boost::lexical_cast< T >( v[i] ); }
    return t;
}

template < typename T, typename V > inline std::vector< T > split_as( const std::string& s, const char* separators, const V& defaults )
{
    const auto& v = split( s, separators, true );
    std::vector< T > t( v.size() < defaults.size() ? defaults.size() : v.size() );
    for( unsigned int i = 0; i < v.size(); ++i ) { t[i] = v[i].empty() && defaults.size() > i ? defaults[i] : boost::lexical_cast< T >( v[i] ); }
    for( unsigned int i = v.size(); i < defaults.size(); ++i ) { t[i] = defaults[i]; }
    return t;
}

template < typename T, typename V > inline std::vector< T > split_as( const std::string& s, char separator, const V& defaults )
{
    const char separators[] = { separator, 0 };
    return split_as< T >( s, &separators[0], defaults );
}

} // namespace impl {

template < typename T > inline std::vector< T > split_as( const std::string& s, const char* separators ) { return impl::split_with_scalar_default( s, separators, boost::optional< T >() ); }

template < typename T > inline std::vector< T > split_as( const std::string& s, const char* separators, const T& default_value ) { return impl::split_with_scalar_default( s, separators, default_value ); }

template < typename T > inline std::vector< T > split_as( const std::string& s, char separator )
{ 
    const char separators[] = { separator, 0 };
    return impl::split_with_scalar_default( s, &separators[0], boost::optional< T >() );
}

template < typename T > inline std::vector< T > split_as( const std::string& s, char separator, const T& default_value )
{ 
    const char separators[] = { separator, 0 };
    return impl::split_with_scalar_default( s, &separators[0], boost::optional< T >( default_value ) );
}

template < typename T > inline std::vector< T > split_as( const std::string& s, const char* separators, const std::vector< T >& defaults ) { return impl::split_as< T >( s, separators, defaults ); }
template < typename T > inline std::vector< T > split_as( const std::string& s, char separator, const std::vector< T >& defaults ) { return impl::split_as< T >( s, separator, defaults ); }
template < typename T, std::size_t N > inline std::vector< T > split_as( const std::string& s, const char* separators, const std::array< T, N >& defaults ) { return impl::split_as< T >( s, separators, defaults ); }
template < typename T, std::size_t N > inline std::vector< T > split_as( const std::string& s, char separator, const std::array< T, N >& defaults ) { return impl::split_as< T >( s, separator, defaults ); }
template < typename T, std::size_t N > inline std::vector< T > split_as( const std::string& s, const char* separators, const boost::array< T, N >& defaults ) { return impl::split_as< T >( s, separators, defaults ); }
template < typename T, std::size_t N > inline std::vector< T > split_as( const std::string& s, char separator, const boost::array< T, N >& defaults ) { return impl::split_as< T >( s, separator, defaults ); }

} // namespace comma {
