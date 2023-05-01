// Copyright (c) 2023 Vsevolod Vlaskine

/// @author Vsevolod Vlaskine

#pragma once

#include <memory>
#include <string>
#include "../visiting/traits.h"
#include "string.h"

namespace comma { namespace visiting {

// todo: add traits for other types

template < std::size_t Size, char Padding > struct traits< comma::packed::string< Size, Padding > >
{
    template < typename Key, class Visitor > static void visit( const Key& k, comma::packed::string< Size, Padding >& p, Visitor& v )
    {
        // todo? quick and dirty for now; should we support nacked pointers in visitors?
        // todo? should we replace Padding with 0?
        std::string s( p.data(), Size );
        v.apply( k, s );
        std::memset( p.data(), Padding, Size );
        std::memcpy( p.data(), &s[0], std::min( s.size(), Size ) );
    }

    template < typename Key, class Visitor > static void visit( const Key& k, const comma::packed::string< Size, Padding >& p, Visitor& v )
    {
        // todo? quick and dirty for now; should we support nacked pointers in visitors?
        // todo? should we replace Padding with 0?
        std::string s( p.data(), Size );
        v.apply( k, s );
    }
};

} } // namespace comma { namespace visiting {
