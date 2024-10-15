// Copyright (c) 2024 Vsevolod Vlaskine
// All rights reserved.

/// @author vsevolod vlaskine

#pragma once

#include "../visiting/traits.h"
#include "serial.h"

namespace comma { namespace visiting {

template <> struct traits< comma::io::serial::port::properties >
{
    template< typename K, typename V > static void visit( const K&, comma::io::serial::port::properties& t, V& v )
    {
        v.apply( "name", t.name );
        v.apply( "baud_rate", t.baud_rate );
    }
    
    template< typename K, typename V > static void visit( const K&, const comma::io::serial::port::properties& t, V& v )
    {
        v.apply( "name", t.name );
        v.apply( "baud_rate", t.baud_rate );
    }
};

} } // namespace comma { namespace visiting { 
