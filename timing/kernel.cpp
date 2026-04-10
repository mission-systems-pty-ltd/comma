// Copyright (c) 2025 Vsevolod Vlaskine
// All rights reserved.

/// @authors rex crisp, vsevolod vlaskine

#include <boost/date_time/posix_time/posix_time.hpp>

namespace comma { namespace timing { namespace kernel {

boost::posix_time::ptime boot_time()
{
    struct timespec real, mono;
    clock_gettime( CLOCK_REALTIME, &real );
    clock_gettime( CLOCK_MONOTONIC, &mono );
    boost::posix_time::ptime real_time = boost::posix_time::from_time_t(real.tv_sec) + boost::posix_time::microseconds(real.tv_nsec / 1000);
    boost::posix_time::time_duration mono_time = boost::posix_time::seconds(mono.tv_sec) + boost::posix_time::microseconds(mono.tv_nsec / 1000);
    return real_time - mono_time;
 }

} } } // namespace comma { namespace timing { namespace kernel {
