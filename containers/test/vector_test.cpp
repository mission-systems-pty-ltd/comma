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
// License along with comma. If not, see <http://www.gnu.org/licenses/>.

#include <gtest/gtest.h>
#include <boost/date_time/posix_time/ptime.hpp>
#include <comma/containers/vector.h>

namespace comma {

TEST( regular_vector, index )
{
    {
        comma::regular_vector< double, int > v( 1.2, 0.5, 10 );
        EXPECT_EQ( -1, v.index( 1 ) );
        EXPECT_EQ( 0, v.index( 1.2 ) );
        EXPECT_EQ( 0, v.index( 1.3 ) );
        EXPECT_EQ( 0, v.index( 1.4999 ) );
        EXPECT_EQ( 2, v.index( 2.2 ) );
        EXPECT_EQ( 2, v.index( 2.3 ) );
        EXPECT_EQ( 2, v.index( 2.4999 ) );
    }
    {
        comma::regular_vector< double, int > v( -1.0, 0.5, 10 );
        EXPECT_EQ( -2, v.index( -1.5000001 ) );
        EXPECT_EQ( -1, v.index( -1.5 ) );
        EXPECT_EQ( -1, v.index( -1.4999999 ) );
        EXPECT_EQ( -1, v.index( -1.0000001 ) );
        EXPECT_EQ( 0, v.index( -1 ) );
        EXPECT_EQ( 0, v.index( -0.9999999 ) );
        EXPECT_EQ( 1, v.index( -0.0000001 ) );
        EXPECT_EQ( 2, v.index( 0 ) );
        EXPECT_EQ( 2, v.index( 0.0000001 ) );
    }
}

} // namespace comma {
