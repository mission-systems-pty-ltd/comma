// This file is part of comma, a generic and flexible library
// Copyright (c) 2011 The University of Sydney
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. Neither the name of the University of Sydney nor the
//    names of its contributors may be used to endorse or promote products
//    derived from this software without specific prior written permission.
//
// NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
// GRANTED BY THIS LICENSE.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
// HOLDERS AND CONTRIBUTORS \"AS IS\" AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
// BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
// IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <gtest/gtest.h>
#include "../../application/command_line_options.h"
#include "../../csv/options.h"

namespace comma {

TEST( options, has_paths )
{
    {
        comma::csv::options csv;
        csv.full_xpath = false;
        csv.fields = "a,b/c,d/e/f,p[0],q[0]/x,s/t/y[0],s/t/y[1]/z";
        EXPECT_TRUE( csv.has_paths( "a" ) );
        EXPECT_TRUE( csv.has_paths( "b" ) );
        EXPECT_TRUE( csv.has_paths( "b/c" ) );
        EXPECT_TRUE( csv.has_paths( "d" ) );
        EXPECT_TRUE( csv.has_paths( "d/e" ) );
        EXPECT_TRUE( csv.has_paths( "d/e/f" ) );
        EXPECT_TRUE( csv.has_paths( "p" ) );
        EXPECT_TRUE( csv.has_paths( "p[0]" ) );
        EXPECT_TRUE( csv.has_paths( "q" ) );
        EXPECT_TRUE( csv.has_paths( "q[0]" ) );
        EXPECT_TRUE( csv.has_paths( "q[0]/x" ) );
        EXPECT_TRUE( csv.has_paths( "s" ) );
        EXPECT_TRUE( csv.has_paths( "s/t" ) );
        EXPECT_TRUE( csv.has_paths( "s/t/y" ) );
        EXPECT_TRUE( csv.has_paths( "s/t/y[0]" ) );
        EXPECT_TRUE( csv.has_paths( "s/t/y[1]" ) );
        EXPECT_TRUE( csv.has_paths( "s/t/y[1]/z" ) );
        EXPECT_TRUE( csv.has_paths( "a,b/c,d/e/f,p[0],q[0]/x,s/t/y[0],s/t/y[1]/z" ) );
        EXPECT_FALSE( csv.has_paths( "aa" ) );
        EXPECT_FALSE( csv.has_paths( "a,blah" ) );
        EXPECT_FALSE( csv.has_paths( "b/cc" ) );
        EXPECT_FALSE( csv.has_paths( "b/c[0]" ) );
        EXPECT_FALSE( csv.has_paths( "b/c,blah" ) );
        EXPECT_FALSE( csv.has_paths( "blah,b/c" ) );
        EXPECT_FALSE( csv.has_paths( "c" ) );
        EXPECT_FALSE( csv.has_paths( "s/t/y/z" ) );
    }
    {
        comma::csv::options csv;
        csv.full_xpath = false;
        csv.fields = "a,b/c,d/e/f,p[0],q[0]/x,s/t/y[0],s/t/y[1]/z";
        EXPECT_TRUE( csv.has_some_of_paths( "a" ) );
        EXPECT_TRUE( csv.has_some_of_paths( "a,blah" ) );
        EXPECT_TRUE( csv.has_some_of_paths( "blah,a" ) );
        EXPECT_TRUE( csv.has_some_of_paths( "b" ) );
        EXPECT_TRUE( csv.has_some_of_paths( "b/c" ) );
        EXPECT_TRUE( csv.has_some_of_paths( "d" ) );
        EXPECT_TRUE( csv.has_some_of_paths( "d/e" ) );
        EXPECT_TRUE( csv.has_some_of_paths( "d/e/f" ) );
        EXPECT_TRUE( csv.has_some_of_paths( "p" ) );
        EXPECT_TRUE( csv.has_some_of_paths( "p[0]" ) );
        EXPECT_TRUE( csv.has_some_of_paths( "q" ) );
        EXPECT_TRUE( csv.has_some_of_paths( "q[0]" ) );
        EXPECT_TRUE( csv.has_some_of_paths( "q[0]/x" ) );
        EXPECT_TRUE( csv.has_some_of_paths( "s" ) );
        EXPECT_TRUE( csv.has_some_of_paths( "s/t" ) );
        EXPECT_TRUE( csv.has_some_of_paths( "s/t/y" ) );
        EXPECT_TRUE( csv.has_some_of_paths( "s/t/y[0]" ) );
        EXPECT_TRUE( csv.has_some_of_paths( "s/t/y[1]" ) );
        EXPECT_TRUE( csv.has_some_of_paths( "s/t/y[1]/z" ) );
        EXPECT_TRUE( csv.has_some_of_paths( "a,b/c,d/e/f,p[0],q[0]/x,s/t/y[0],s/t/y[1]/z" ) );
        EXPECT_FALSE( csv.has_some_of_paths( "s/t/y/z" ) );
        EXPECT_FALSE( csv.has_some_of_paths( "s/t/y[2]/z" ) );
        EXPECT_FALSE( csv.has_some_of_paths( "aa" ) );
        EXPECT_FALSE( csv.has_some_of_paths( "b/cc" ) );
        EXPECT_FALSE( csv.has_some_of_paths( "blah" ) );
        EXPECT_FALSE( csv.has_some_of_paths( "blah,b/c[5]" ) );
        EXPECT_FALSE( csv.has_some_of_paths( "c" ) );
    }
}

TEST( options, aliases )
{
    {
        comma::csv::options csv( comma::command_line_options( std::vector< std::string >{ "" } ), std::unordered_map< std::string, std::string >() );
        EXPECT_EQ( csv.fields, "" );
    }
    {
        comma::csv::options csv( comma::command_line_options( std::vector< std::string >{ "" } ), { { "b", "x/y/b" } }, "a,b,b" );
        EXPECT_EQ( csv.fields, "a,x/y/b,x/y/b" );
    }
    {
        comma::csv::options csv( comma::command_line_options( std::vector< std::string >{ "", "--fields=a,b,c" } ), std::unordered_map< std::string, std::string >() );
        EXPECT_EQ( csv.fields, "a,b,c" );
    }
    {
        comma::csv::options csv( comma::command_line_options( std::vector< std::string >{ "", "--fields=,b,c" } ), { { "b", "x/y/b" } } );
        EXPECT_EQ( csv.fields, ",x/y/b,c" );
    }
    {
        comma::csv::options csv( comma::command_line_options( std::vector< std::string >{ "", "--fields=a,b," } ), { { "b", "x/y/b" } } );
        EXPECT_EQ( csv.fields, "a,x/y/b," );
    }
    {
        comma::csv::options csv( comma::command_line_options( std::vector< std::string >{ "", "--fields=a,,b" } ), { { "b", "x/y/b" } } );
        EXPECT_EQ( csv.fields, "a,,x/y/b" );
    }
    {
        comma::csv::options csv( comma::command_line_options( std::vector< std::string >{ "", "--fields=a,b,c" } ), { { "b", "x/y/b" } } );
        EXPECT_EQ( csv.fields, "a,x/y/b,c" );
    }
    // todo: more tests
}

} // namespace comma {
