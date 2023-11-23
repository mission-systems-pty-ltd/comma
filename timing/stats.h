// Copyright (c) 2023 Vsevolod Vlaskine
// All rights reserved.

/// @author vsevolod vlaskine

#pragma once

#include <chrono>
#include "../math/exponential_moving_average.h"

namespace comma { namespace timing {

class stats
{
    public:
        stats( double ema_alpha = 0.5, comma::uint64 ema_initial_count = 1 ): _ema( ema_alpha, ema_initial_count ) {}

        stats& operator++() { return operator+=( std::chrono::system_clock::now() ); }

        stats& operator+=( const std::chrono::time_point< std::chrono::system_clock >& t );

        comma::uint64 count() const { return _ema.count(); }

        double rate() const { return 1. / _ema(); }

        double ema() const { return _ema(); }

        double min() const { return _min; }

        double max() const { return _max; }

    private:
        std::chrono::time_point< std::chrono::system_clock > _t;
        math::exponential_moving_average< double > _ema;
        double _min{0};
        double _max{0};
};

inline stats& stats::operator+=( const std::chrono::time_point< std::chrono::system_clock >& t ) // todo: move to cpp file
{
    if( _t.time_since_epoch() > std::chrono::seconds( 0 ) )
    {
        double d = double( std::chrono::duration_cast< std::chrono::microseconds >( t - _t ).count() ) / 1000000;
        if( _ema.count() == 0 ) { _min = _max = d; }
        else { if( d < _min ) { _min = d; } else if( d > _max ) { _max = d; } }
        _ema += double( std::chrono::duration_cast< std::chrono::microseconds >( t - _t ).count() ) / 1000000; // quick and dirty for now
    }
    _t = t;
    return *this;
}

} } // namespace comma { namespace timing {
