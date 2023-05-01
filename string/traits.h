// Copyright (c) 2023 Vsevolod Vlaskine

/// @author Vsevolod Vlaskine

#pragma once

#include <string>
#include "../base/exception.h"
#include "../visiting/apply.h"
#include "../visiting/traits.h"
#include "choice.h"

namespace comma { namespace visiting {

template < typename Derived, typename Base > struct traits< comma::strings::choice< Derived, Base > >
{
    typedef comma::strings::choice< Derived, Base > choice_t;

    template < typename Key, class Visitor > static void visit( const Key& k, choice_t& p, Visitor& v )
    {
        comma::visiting::apply( v, static_cast< Base& >( p ) );
        choice_t::assert_valid( std::string( p ) );
    }

    template < typename Key, class Visitor > static void visit( const Key& k, const choice_t& p, Visitor& v )
    {
        comma::visiting::apply( v, static_cast< const Base& >( p ) );
    }
};

} } // namespace comma { namespace visiting {
