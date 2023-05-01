// Copyright (c) 2023 Vsevolod Vlaskine

#pragma once

#include <string>
#include "../base/exception.h"

namespace comma {

template < typename Derived, typename S = std::string >
struct choice: public S
{
    choice( typename Derived::values rhs = static_cast< typename Derived::values >( 0 ) ): S( Derived::choices()[rhs] ) {}
    choice( const std::string& rhs ) { operator=( rhs ); }
    choice& operator=( const std::string& rhs ) { COMMA_ASSERT( valid( rhs ), "expected value; got: '" << rhs << "'" ); operator=( rhs ); return *this; }
    typename Derived::values to_enum() const;
    static bool valid( const std::string& rhs );
};

template < typename Derived, typename S >
typename Derived::values choice< Derived, S >::to_enum() const
{
    unsigned int i = 0;
    for( const auto& c: Derived::choices() ) { if( *this == c ) { return static_cast< typename Derived::values >( i ); } ++i; }
    COMMA_THROW( comma::exception, "could not convert to enum value: '" << std::string( *this ) << "'" ); // in theory never here
}

template < typename Derived, typename S >
bool choice< Derived, S >::valid( const std::string& rhs )
{
    unsigned int i = 0;
    for( const auto& c: Derived::choices() ) { if( rhs == c ) { return true; } ++i; }
    return false;
}

} // namespace comma {
