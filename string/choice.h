// Copyright (c) 2023 Vsevolod Vlaskine

#pragma once

#include <string>
#include "../base/exception.h"
#include "string.h"

namespace comma { namespace strings {

template < typename Derived, typename Base = std::string >
struct choice: public Base
{
    typedef Base base_t;
    choice( typename Derived::values rhs = static_cast< typename Derived::values >( 0 ) ): Base( Derived::choices()[rhs] ) {}
    choice( const std::string& rhs ) { operator=( rhs ); }
    choice& operator=( const std::string& rhs ) { assert_valid( rhs ); Base::operator=( rhs ); return *this; }
    typename Derived::values to_enum() const;
    static bool valid( const std::string& rhs );
    static void assert_valid( const std::string& rhs );
    bool valid() const { return valid( std::string( *this ) ); } // quick and dirty for now
    void assert_valid() const { assert_valid( std::string( *this ) ); } // quick and dirty for now
};

template < typename Derived, typename Base >
typename Derived::values choice< Derived, Base >::to_enum() const
{
    unsigned int i = 0;
    for( const auto& c: Derived::choices() ) { if( *this == c ) { return static_cast< typename Derived::values >( i ); } ++i; }
    COMMA_THROW( comma::exception, "could not convert to enum value: '" << std::string( *this ) << "'" ); // in theory never here
}

template < typename Derived, typename Base >
bool choice< Derived, Base >::valid( const std::string& rhs )
{
    unsigned int i = 0;
    for( const auto& c: Derived::choices() ) { if( rhs == c ) { return true; } ++i; }
    return false;
}

template < typename Derived, typename Base >
void choice< Derived, Base >::assert_valid( const std::string& rhs )
{
    COMMA_ASSERT( valid( rhs ), "expected one of: " << comma::join( Derived::choices(), ',' ) << "; got: '" << rhs << "'" );
}

} } // namespace comma { namespace strings {
