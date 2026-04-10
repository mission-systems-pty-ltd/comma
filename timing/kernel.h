// Copyright (c) 2025 Vsevolod Vlaskine
// All rights reserved.

/// @authors rex crisp, vsevolod vlaskine

#pragma once

#include <boost/date_time/posix_time/posix_time.hpp>

namespace comma { namespace timing { namespace kernel {

boost::posix_time::ptime boot_time();

} } } // namespace comma { namespace timing { namespace kernel {
