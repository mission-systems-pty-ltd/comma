// Copyright (c) 2024 Vsevolod Vlaskine
// All rights reserved.

/// @author vsevolod vlaskine

#include <boost/date_time/posix_time/posix_time.hpp>
#include "../base/exception.h"
#include "conversions.h"
#include "epoch.h"

namespace comma { namespace timing {

std::string to_iso_string( boost::posix_time::ptime t, unsigned int fraction_digits, bool strict )
{
    std::string s = boost::posix_time::to_iso_string( t );
    unsigned int size = 16 + fraction_digits;
    if( t.is_not_a_date_time() || t.is_neg_infinity() || t.is_infinity() ) { COMMA_THROW_IF( strict, "expected valid time; got: '" << s << "'" ); return s; }
    return s.size() < size ? s + std::string( '0', size - s.size() ) : s.substr( 0, size );
}

boost::posix_time::ptime as_ptime( std::chrono::system_clock::time_point t )
{
    boost::posix_time::ptime p = boost::posix_time::from_time_t( std::chrono::system_clock::to_time_t( t ) );
    p += boost::posix_time::microseconds( std::chrono::duration_cast< std::chrono::microseconds >( t.time_since_epoch() % std::chrono::seconds( 1 ) ).count() );
    return p;
}

std::chrono::system_clock::time_point as_time_point( const boost::posix_time::ptime& p )
{
    COMMA_THROW_IF( p.is_infinity(), "conversion of 'infinity' chrono::...::time_point: not implemented" );
    COMMA_THROW_IF( p.is_neg_infinity(), "conversion of 'neg-infinity' chrono::...::time_point: not implemented" );
    if( p.is_not_a_date_time() ) { return std::chrono::system_clock::from_time_t( 0 ); }
    return std::chrono::system_clock::from_time_t( 0 ) + std::chrono::microseconds( ( p - timing::epoch_time() ).total_microseconds() );
}

boost::posix_time::ptime from_iso_string( const std::string& s )
{
    if ( s == "not-a-date-time" ) { return boost::posix_time::not_a_date_time; }
    else if ( s == "+infinity" || s == "+inf" || s == "inf" ) { return boost::posix_time::pos_infin; }
    else if ( s == "-infinity" || s == "-inf" ) { return boost::posix_time::neg_infin; }
    else return boost::posix_time::from_iso_string( s );
}

} } // namespace comma { namespace timing {
