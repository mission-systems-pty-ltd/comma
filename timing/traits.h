// Copyright (c) 2023 Vsevolod Vlaskine
// All rights reserved.

/// @author vsevolod vlaskine

#include "../timing/stats.h"
#include "../timing/timestamped.h"
#include "../visiting/traits.h"

namespace comma { namespace visiting {

template <> struct traits< comma::timing::stats > // quick and dirty
{
    template < typename Key, class Visitor > static void visit( const Key& k, const comma::timing::stats& p, Visitor& v )
    {
        v.apply( "elapsed", p.elapsed() );
        v.apply( "count", p.count() );
        v.apply( "rate", p.rate() );
        v.apply( "min", p.min() );
        v.apply( "max", p.max() );
        v.apply( "mean", p.ema() );
    }
};

template < typename T > struct traits< comma::timestamped< T > >
{
    template< typename K, typename V > static void visit( const K&, comma::timestamped< T >& p, V& v )
    {
        v.apply( "t", p.t );
        v.apply( "data", p.data );
    }

    template< typename K, typename V > static void visit( const K&, const comma::timestamped< T >& p, V& v )
    {
        v.apply( "t", p.t );
        v.apply( "data", p.data );
    }
};

} } // namespace comma { namespace visiting {
