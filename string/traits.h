// Copyright (c) 2023 Vsevolod Vlaskine

/// @author Vsevolod Vlaskine

#pragma once

#include <string>
#include "../base/exception.h"
#include "../visiting/traits.h"
#include "choice.h"

namespace comma { namespace visiting {

template < typename Derived, typename Base > struct traits< comma::strings::choice< Derived, Base > >
{
    typedef comma::strings::choice< Derived, Base > choice_t;

    template < typename Key, class Visitor > static void visit( const Key& k, choice_t& p, Visitor& v )
    {
        std::string s( p );
        v.apply( k, s );
        choice_t::assert_valid( s );
        p = s;
    }

    template < typename Key, class Visitor > static void visit( const Key& k, const choice_t& p, Visitor& v )
    {
        v.apply( k, static_cast< const Base& >( p ) );
    }
};

} } // namespace comma { namespace visiting {
