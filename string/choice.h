// Copyright (c) 2023 Vsevolod Vlaskine

#pragma once

#include <string>
#include <vector>
#include "../base/exception.h"
#include "string.h"

namespace comma { namespace strings {

template < typename Derived, typename Base = std::string >
struct choice: public Base
{
    typedef Base base_t;
    choice( typename Derived::values rhs = static_cast< typename Derived::values >( 0 ) ): Base( Derived::choices()[static_cast< unsigned int >( rhs )] ) {}
    choice( const std::string& rhs ) { operator=( rhs ); }
    choice& operator=( const std::string& rhs ) { assert_valid( rhs ); Base::operator=( rhs ); return *this; }
    typename Derived::values to_enum() const;
    static bool valid( const std::string& rhs );
    static void assert_valid( const std::string& rhs );
    bool valid() const { return valid( std::string( *this ) ); } // quick and dirty for now
    void assert_valid() const { assert_valid( std::string( *this ) ); } // quick and dirty for now
    operator typename Derived::values() const { return to_enum(); }
};

template < typename Enum >
Enum make_choice( const std::string& name, const std::vector< std::string >& choices ); // convenience function, quick and dirty for now

namespace impl {

template < typename Enum, typename T, typename V >
inline Enum make_choice( const T& name, const V& choices ) // quick and dirty for now
{
    unsigned int i = 0;
    for( const auto& c: choices ) { if( name == c ) { return static_cast< Enum >( i ); } ++i; }
    COMMA_THROW( comma::exception, "could not convert to enum value: '" << name << "'" ); // in theory never here
}

} // namespace impl {

template < typename Enum >
inline Enum make_choice( const std::string& name, const std::vector< std::string >& choices )
{
    return impl::make_choice< Enum >( name, choices );
}

template < typename Derived, typename Base >
inline typename Derived::values choice< Derived, Base >::to_enum() const
{
    return impl::make_choice< typename Derived::values >( static_cast< const Base& >( *this ), Derived::choices() );
}

template < typename Derived, typename Base >
inline bool choice< Derived, Base >::valid( const std::string& rhs )
{
    unsigned int i = 0;
    for( const auto& c: Derived::choices() ) { if( rhs == c ) { return true; } ++i; }
    return false;
}

template < typename Derived, typename Base >
inline void choice< Derived, Base >::assert_valid( const std::string& rhs )
{
    COMMA_ASSERT( valid( rhs ), "expected one of: " << comma::join( Derived::choices(), ',' ) << "; got: '" << rhs << "'" );
}

} } // namespace comma { namespace strings {
