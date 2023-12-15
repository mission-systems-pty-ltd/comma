// Copyright (c) 2023 Vsevolod Vlaskine
// All rights reserved.

/// @author vsevolod vlaskine

#include "duration.h"
#include "epoch.h"

namespace comma { namespace timing {

boost::posix_time::ptime from_seconds_since_epoch( double seconds, boost::gregorian::date e )
{
    return boost::posix_time::ptime( e, duration::from_seconds( seconds ) );
}

} } // namespace comma { namespace timing {
