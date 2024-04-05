// Copyright (c) 2024 Mission Systems Pty Ltd

#pragma once

#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace comma { namespace timing {

namespace tai {

// For conversion of fast streaming data you probably want to just get the leap
// seconds once then apply that offset to all the data (utc = tai - leap_seconds).
// This will not be accurate if the data crosses a leap-second boundary but is much faster.
int leap_seconds( const boost::posix_time::ptime& time, bool time_is_utc = true );

// Otherwise use the from/to functions which are accurate across boundaries.
// Although note that boost doesn't understand UTC times of 23:59:60.
// It thinks it's the same as 00:00:00 even if it aligns with a leap-second.
boost::posix_time::ptime from_utc( const boost::posix_time::ptime& utc );
boost::posix_time::ptime to_utc( const boost::posix_time::ptime& tai );

} // namespace tai {

inline int leap_seconds( const boost::posix_time::ptime& time, bool time_is_utc = true ) { return tai::leap_seconds( time, time_is_utc ); }

} } // namespace comma { namespace timing {
