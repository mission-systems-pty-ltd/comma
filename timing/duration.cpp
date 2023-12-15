// Copyright (c) 2023 Vsevolod Vlaskine
// All rights reserved.

/// @author vsevolod vlaskine

#include "duration.h"

namespace comma { namespace timing { namespace duration {

boost::posix_time::time_duration from_seconds( double seconds )
{
    long long s = seconds;
    int microseconds = ::ceil( ( seconds - s ) * 1000000 - 0.5 ); //int microseconds = ::round( ( d - seconds ) * 1000000 ); // although ::round() is slow, have to round, since lexical cast has floating point jitter, e.g. try: boost::lexical_cast< double >( "1369179610.752231000" );
    return boost::posix_time::time_duration( boost::posix_time::seconds( seconds ) + boost::posix_time::microseconds( microseconds ) );
}

} } } // namespace comma { namespace timing { namespace duration {
