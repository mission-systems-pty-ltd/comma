// Copyright (c) 2024 Vsevolod Vlaskine

#pragma once

#include "../../visiting/traits.h"
#include "index.h"

namespace comma { namespace visiting {

template < unsigned int D, typename T > struct traits< comma::containers::multidimensional::index< D, T > >
{
    typedef comma::containers::multidimensional::index< D, T > value_t;

    template < typename Key, class Visitor > static void visit( const Key& k, value_t& p, Visitor& v )
    {
        comma::visiting::traits< std::array< T, D > >::visit( k, static_cast< std::array< T, D >& >( p ), v );
    }

    template < typename Key, class Visitor > static void visit( const Key& k, const value_t& p, Visitor& v )
    {
        comma::visiting::traits< std::array< T, D > >::visit( k, static_cast< const std::array< T, D >& >( p ), v );
    }
};

} } // namespace comma { namespace visiting {
