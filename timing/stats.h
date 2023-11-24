// Copyright (c) 2023 Vsevolod Vlaskine
// All rights reserved.

/// @author vsevolod vlaskine

#pragma once

#include <chrono>
#include <iostream>
#include <string>
#include "../math/exponential_moving_average.h"

namespace comma { namespace timing {

class stats
{
    public:
        typedef std::chrono::time_point< std::chrono::system_clock > time_type;

        typedef decltype( time_type() - time_type() ) duration_type;

        stats( double ema_alpha = 0.5, comma::uint64 ema_initial_count = 1 ): _ema( ema_alpha, ema_initial_count ), _previous_output_time( std::chrono::system_clock::now() ) {}

        stats& operator+=( const time_type& t );

        stats& operator++() { return operator+=( std::chrono::system_clock::now() ); }

        stats& touch( const time_type& t ) { _t = t; return *this; }

        stats& touch() { return touch( std::chrono::system_clock::now() ); }

        comma::uint64 count() const { return _ema.count(); }

        double rate() const { return 1. / _ema(); }

        double ema() const { return _ema(); }

        double min() const { return _min; }

        double max() const { return _max; }

        void output( std::ostream& os = std::cerr, const std::string& prefix = "" );
    
        void output( unsigned int count = 1, std::ostream& os = std::cerr, const std::string& prefix = "" );

        void output_every( const stats::duration_type& d, std::ostream& os = std::cerr, const std::string& prefix = "" );

        double elapsed() const { return double( std::chrono::duration_cast< std::chrono::microseconds >( _t - _start ).count() ) / 1000000; }

    private:
        time_type _start;
        time_type _t;
        time_type _previous_output_time;
        math::exponential_moving_average< double > _ema;
        double _min{0};
        double _max{0};
};

inline stats& stats::operator+=( const stats::time_type& t ) // todo: move to cpp file
{
    if( _start.time_since_epoch() == std::chrono::seconds( 0 ) )
    {
        _start = t;
    }
    else
    {
        double d = double( std::chrono::duration_cast< std::chrono::microseconds >( t - _t ).count() ) / 1000000;
        if( _ema.count() == 0 ) { _min = _max = d; }
        else { if( d < _min ) { _min = d; } else if( d > _max ) { _max = d; } }
        _ema += double( std::chrono::duration_cast< std::chrono::microseconds >( t - _t ).count() ) / 1000000; // quick and dirty for now
    }
    _t = t;
    return *this;
}

inline void stats::output( std::ostream& os, const std::string& prefix )
{
    // freaking hate chrono! os << prefix << "start=" << _start << ";elapsed=" << elapsed() << ";count=" << _ema.count() << ";rate=" << rate() << ";intervals/min=" << _min << ";intervals/max=" << _max << ";intervals/mean=" << _ema() << std::endl;
    if( _ema.count() == 0 ) { os << prefix << "elapsed=" << elapsed() << ";count=" << _ema.count() << ";rate=nan;intervals/min=nan;intervals/max=nan;intervals/mean=nan" << std::endl; }
    else { os << prefix << "elapsed=" << elapsed() << ";count=" << _ema.count() << ";rate=" << rate() << ";intervals/min=" << _min << ";intervals/max=" << _max << ";intervals/mean=" << _ema() << std::endl; }
}

inline void stats::output( unsigned int c, std::ostream& os, const std::string& prefix )
{
    if( count() > 0 && count() % c == 0 ) { output( os, prefix ); }
}

inline void stats::output_every( const stats::duration_type& d, std::ostream& os, const std::string& prefix )
{
    auto now = std::chrono::system_clock::now();
    if( ( now - _previous_output_time ) < d ) { return; }
    output( os, prefix );
    _previous_output_time = now;
}


} } // namespace comma { namespace timing {
