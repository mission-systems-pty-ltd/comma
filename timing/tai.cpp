// Copyright (c) 2024 Mission Systems Pty Ltd
//
// Data extracted from https://hpiers.obspm.fr/eoppc/bul/bulc/UTC-TAI.history
// which is linked to from https://www.iers.org/IERS/EN/Publications/Bulletins/bulletins.html
//
// For an overview see https://en.wikipedia.org/wiki/Leap_second
//
// Note that boost::posix_time doesn't really support leap seconds. It treats
// 19720630T235960 as identical to 19720701T000000. You can see this with:
//   $ echo 19720630T235959 | csv-time --to seconds
//   78796799
//   $ echo 19720630T235960 | csv-time --to seconds
//   78796800
//   $ echo 19720701T000000 | csv-time --to seconds
//   78796800
//
// There should be an extra second there
//
// Note that C++20 introduces std::chrono::tai_clock
// https://en.cppreference.com/w/cpp/chrono/tai_clock

/// @author dave jennings

#include "tai.h"
#include <utility>
#include <vector>

namespace comma { namespace timing { namespace tai {

typedef std::pair< boost::posix_time::ptime, int > leap_seconds_entry;

using boost::posix_time::ptime;
using boost::gregorian::date;
using boost::date_time::Jan;
using boost::date_time::Jul;

static std::vector< leap_seconds_entry > leap_seconds_table = {
    leap_seconds_entry(  boost::date_time::neg_infin, 0 ),
    leap_seconds_entry( ptime( date( 1972, Jan, 1 )), 10 ),
    leap_seconds_entry( ptime( date( 1972, Jul, 1 )), 11 ),
    leap_seconds_entry( ptime( date( 1973, Jan, 1 )), 12 ),
    leap_seconds_entry( ptime( date( 1974, Jan, 1 )), 13 ),
    leap_seconds_entry( ptime( date( 1975, Jan, 1 )), 14 ),
    leap_seconds_entry( ptime( date( 1976, Jan, 1 )), 15 ),
    leap_seconds_entry( ptime( date( 1977, Jan, 1 )), 16 ),
    leap_seconds_entry( ptime( date( 1978, Jan, 1 )), 17 ),
    leap_seconds_entry( ptime( date( 1979, Jan, 1 )), 18 ),
    leap_seconds_entry( ptime( date( 1980, Jan, 1 )), 19 ),
    leap_seconds_entry( ptime( date( 1981, Jul, 1 )), 20 ),
    leap_seconds_entry( ptime( date( 1982, Jul, 1 )), 21 ),
    leap_seconds_entry( ptime( date( 1983, Jul, 1 )), 22 ),
    leap_seconds_entry( ptime( date( 1985, Jul, 1 )), 23 ),
    leap_seconds_entry( ptime( date( 1988, Jan, 1 )), 24 ),
    leap_seconds_entry( ptime( date( 1990, Jan, 1 )), 25 ),
    leap_seconds_entry( ptime( date( 1991, Jan, 1 )), 26 ),
    leap_seconds_entry( ptime( date( 1992, Jul, 1 )), 27 ),
    leap_seconds_entry( ptime( date( 1993, Jul, 1 )), 28 ),
    leap_seconds_entry( ptime( date( 1994, Jul, 1 )), 29 ),
    leap_seconds_entry( ptime( date( 1996, Jan, 1 )), 30 ),
    leap_seconds_entry( ptime( date( 1997, Jul, 1 )), 31 ),
    leap_seconds_entry( ptime( date( 1999, Jan, 1 )), 32 ),
    leap_seconds_entry( ptime( date( 2006, Jan, 1 )), 33 ),
    leap_seconds_entry( ptime( date( 2009, Jan, 1 )), 34 ),
    leap_seconds_entry( ptime( date( 2012, Jul, 1 )), 35 ),
    leap_seconds_entry( ptime( date( 2015, Jul, 1 )), 36 ),
    leap_seconds_entry( ptime( date( 2017, Jan, 1 )), 37 ),
    leap_seconds_entry(  boost::date_time::pos_infin, 37 )
};

// The switch-over times are in UTC. That's how we get a time of 23:59:60.
// So when working out the leap_second offset we need the UTC timestamp.
// See https://en.wikipedia.org/wiki/Leap_second#Process
static std::vector< leap_seconds_entry >::reverse_iterator lookup_table( const boost::posix_time::ptime& time, bool time_is_utc )
{
    // Timestamps are likely to be recent so look backwards through the table to
    // find the right entry
    std::vector< leap_seconds_entry >::reverse_iterator riter;
    for( riter = leap_seconds_table.rbegin(); riter != leap_seconds_table.rend(); ++riter )
    {
        boost::posix_time::ptime utc = ( time_is_utc ? time : time - boost::posix_time::seconds( riter->second ));
        if( utc >= riter->first ) { break; }
    }
    return riter;
}

int leap_seconds( const boost::posix_time::ptime& time, bool time_is_utc )
{
    return lookup_table( time, time_is_utc )->second;
}

std::pair< int, boost::posix_time::ptime > leap_seconds_with_valid_time( const boost::posix_time::ptime& time, bool time_is_utc )
{
    std::vector< leap_seconds_entry >::reverse_iterator riter = lookup_table( time, time_is_utc );
    return std::pair< int, boost::posix_time::ptime >( riter->second, ( riter - 1 )->first );
}

boost::posix_time::ptime from_utc( const boost::posix_time::ptime& utc )
{
    return utc + boost::posix_time::seconds( leap_seconds( utc, true ));
}

boost::posix_time::ptime to_utc( const boost::posix_time::ptime& tai )
{
    return tai - boost::posix_time::seconds( leap_seconds( tai, false ));
}

} } } // namespace comma { namespace timing { namespace tai {
