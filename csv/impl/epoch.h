// This file is part of comma, a generic and flexible library 
// for robotics research.
//
// Copyright (C) 2011 The University of Sydney
//
// comma is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// comma is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License 
// for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with comma. If not, see <http://www.gnu.org/licenses/>.auto

/// @author vsevolod vlaskine

#ifndef COMMA_CSV_IMPL_EPOCH_H_
#define COMMA_CSV_IMPL_EPOCH_H_

#include <boost/date_time/posix_time/posix_time.hpp>

namespace comma { namespace csv { namespace impl {

static const boost::gregorian::date epoch( 1970, 1, 1 );

} } } // namespace comma { namespace csv { namespace impl {

#endif // #ifndef COMMA_CSV_IMPL_EPOCH_H_
