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

        double rate() const { return _ema.count() > 0 ? 1. / _ema() : 0; }

        double ema() const { return _ema.count() > 0 ? _ema() : 0; }

        double min() const { return _min; }

        double max() const { return _max; }

        double elapsed() const;

        void output( std::ostream& os = std::cerr, const std::string& prefix = "", bool csv = false );
    
        void output( unsigned int count = 1, std::ostream& os = std::cerr, const std::string& prefix = "", bool csv = false );

        void output_every( const stats::duration_type& d, std::ostream& os = std::cerr, const std::string& prefix = "", bool csv = false );

    private:
        math::exponential_moving_average< double > _ema;
        time_type _start;
        time_type _t;
        time_type _previous_output_time;
        double _min{0};
        double _max{0};
};

} } // namespace comma { namespace timing {
