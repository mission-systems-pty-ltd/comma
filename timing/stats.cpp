// Copyright (c) 2023 Vsevolod Vlaskine
// All rights reserved.

/// @author vsevolod vlaskine

// introduces circular dependencies: #include "../csv/ascii.h" // quick and dirty
#include "../name_value/ptree.h"
#include "../timing/stats.h"
//#include "../timing/traits.h"

namespace comma { namespace timing {

stats& stats::operator+=( const stats::time_type& t ) // todo: move to cpp file
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

double stats::elapsed() const
{ 
    return double( std::chrono::duration_cast< std::chrono::microseconds >( _t - _start ).count() ) / 1000000;
}

void stats::output( std::ostream& os, const std::string& prefix, bool csv ) // todo: template on prefix
{
    // freaking hate chrono! os << prefix << "start=" << _start << ";elapsed=" << elapsed() << ";count=" << _ema.count() << ";rate=" << rate() << ";intervals/min=" << _min << ";intervals/max=" << _max << ";intervals/mean=" << _ema() << std::endl;
    auto p = os.precision();
    os.setf( std::ios::fixed, std::ios::floatfield );
    os << std::setprecision( 6 ) << prefix;
    std::cerr << std::setprecision( 6 );
    if( csv ) // quick and dirty for now to avoid circular dependencies; todo? csv::timing::stats wrapper or something along those lines
    {
        //static comma::csv::ascii< stats > ascii; // introduces circular dependencies
        //os << ascii.put( *this ) << std::endl;
        os << elapsed() << ',' << count() << ',' << rate() << ',' << min() << ',' << max() << ',' << ema() << std::endl;
    }
    else
    {
        // todo! fix! to_ptree: eventually calls ptree.put() which does lexical cast at very high precision
        //                      the solution: parametrize on precision and/or provide translator for putting value (or getting)
        // boost::property_tree::ptree t;
        // to_ptree to( t );
        // visiting::apply( to, *this );
        // comma::property_tree::to_path_value( os, t, comma::property_tree::disabled, '=', ';', xpath(), true );
        // os << std::endl;
        os << prefix << "elapsed=" << elapsed() << ";count=" << _ema.count() << ";rate=" << rate() << ";intervals/min=" << _min << ";intervals/max=" << _max << ";intervals/mean=" << ema() << std::endl;
    }
    os << std::setprecision( p ); // todo! not excetion-safe
}

void stats::output( unsigned int c, std::ostream& os, const std::string& prefix, bool csv )
{
    if( count() > 0 && count() % c == 0 ) { output( os, prefix, csv ); }
}

void stats::output_every( const stats::duration_type& d, std::ostream& os, const std::string& prefix, bool csv )
{
    auto now = std::chrono::system_clock::now();
    if( ( now - _previous_output_time ) < d ) { return; }
    output( os, prefix, csv );
    _previous_output_time = now;
}

} } // namespace comma { namespace timing {
