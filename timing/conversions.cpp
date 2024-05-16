// Copyright (c) 2024 Vsevolod Vlaskine
// All rights reserved.

/// @author vsevolod vlaskine

#include "../base/exception.h"
#include "conversions.h"

namespace comma { namespace timing {

std::string to_iso_string( boost::posix_time::ptime t, unsigned int fraction_digits, bool strict )
{
    std::string s = boost::posix_time::to_iso_string( t );
    unsigned int size = 16 + fraction_digits;
    if( t.is_not_a_date_time() || t.is_neg_infinity() || t.is_infinity() ) { COMMA_THROW_IF( strict, "expected valid time; got: '" << s << "'" ); return s; }
    return s.size() < size ? s + std::string( '0', size - s.size() ) : s.substr( 0, size );
}

} } // namespace comma { namespace timing {
