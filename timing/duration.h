// Copyright (c) 2023 Vsevolod Vlaskine
// All rights reserved.

/// @author vsevolod vlaskine

#pragma once

#include <boost/date_time/posix_time/posix_time.hpp>

namespace comma { namespace timing { namespace duration {

boost::posix_time::time_duration from_seconds( double seconds );

} } } // namespace comma { namespace timing { namespace duration {

