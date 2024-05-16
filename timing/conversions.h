// Copyright (c) 2024 Vsevolod Vlaskine
// All rights reserved.

/// @author vsevolod vlaskine

#pragma once

#include <boost/date_time/posix_time/posix_time.hpp>
#include "duration.h"

namespace comma { namespace timing {

/// same as boost::posix_time::to_iso_string, but pads second fractions with zeroes
/// @param t: time
/// @param fraction_digits: number of second fraction digits
/// @param strict: throw on uninitialised time and infinity
std::string to_iso_string( boost::posix_time::ptime t, unsigned int fraction_digits = 6, bool strict = false );

} } // namespace comma { namespace timing {
