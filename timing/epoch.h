// Copyright (c) 2023 Vsevolod Vlaskine
// All rights reserved.

/// @author vsevolod vlaskine

#pragma once

#include <boost/date_time/posix_time/posix_time.hpp>

namespace comma { namespace timing {

const boost::gregorian::date epoch( 1970, 1, 1 );

boost::posix_time::ptime from_seconds_since_epoch( double seconds, boost::gregorian::date e = timing::epoch );

} } // namespace comma { namespace timing {

