// Copyright (c) 2023 Vsevolod Vlaskine

/// @author vsevolod vlaskine

#pragma once

#include <array>

namespace comma { namespace impl {

template < typename T > struct array_traits;

template < typename T, std::size_t Size > struct array_traits< std::array< T, Size > >;

inline int negative_flooring() { return static_cast< int >( -1.5 ) == -1 ? -1 : static_cast< int >( -1.5 ) == -2 ? 0 : 0; }

inline int positive_flooring() { return static_cast< int >( 1.5 ) == 1 ? 0 : static_cast< int >( 1.5 ) == 2 ? -1 : -1; }

template < typename T, std::size_t Size > struct array_traits< std::array< T, Size > >
{
    enum { size = Size };

    static std::array< T, Size > subtract( const std::array< T, Size >& lhs, const std::array< T, Size >& rhs )
    {
        std::array< T, Size > d;
        for( unsigned int i = 0; i < Size; ++i ) { d[i] = lhs[i] - rhs[i]; }
        return d;
    }

    static std::array< T, Size > divide( const std::array< T, Size >& lhs, const std::array< T, Size >& rhs )
    {
        std::array< T, Size > d;
        for( unsigned int i = 0; i < Size; ++i ) { d[i] = lhs[i] / rhs[i]; }
        return d;
    }

    static std::array< T, Size > zero()
    {
        std::array< T, Size > d;
        for( unsigned int i = 0; i < Size; ++i ) { d[i] = T( 0 ); }
        return d;
    }
};

} } // namespace comma { namespace impl {
