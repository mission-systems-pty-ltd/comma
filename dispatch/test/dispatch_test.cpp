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


#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "../dispatched.h"
#include <gtest/gtest.h>

namespace comma { namespace test {

struct alpha : public dispatch::dispatched< alpha > {};
struct beta : public dispatch::dispatched< beta > {};

class base
{
    public:
        base() : invoked_( false ) {}
        bool invoked() const { return invoked_; }
        const std::string& value() const { invoked_ = false; return value_; }
        void invoke( const std::string& s ) { invoked_ = true; value_ = s; }
    
    private:
        mutable bool invoked_;
        std::string value_;
};

struct multi_handler : public dispatch::handler_of< alpha >
             , public dispatch::handler_of< beta >
             , public dispatch::handler_of_const< beta >
             , public base
{
    void handle( alpha& ) { invoke( "d: got a" ); }
    void handle( beta& ) { invoke( "d: got b" ); }
    void handle( const beta& ) { invoke( "d: got const b" ); }
};

TEST( dispatch, test_basics )
{
    alpha a;
    beta b;
    multi_handler d;
    dispatch::dispatched_base& aref( a );
    dispatch::dispatched_base& bref( b );
    const dispatch::dispatched_base& const_bref( b );
    dispatch::handler& dref( d );
    EXPECT_TRUE( !d.invoked() );
    aref.dispatch_to( dref );
    EXPECT_TRUE( d.invoked() );
    EXPECT_EQ( d.value(), "d: got a" );
    bref.dispatch_to( dref );
    EXPECT_TRUE( d.invoked() );
    EXPECT_EQ( d.value(), "d: got b" );
    const_bref.dispatch_to( dref );
    EXPECT_TRUE( d.invoked() );
    EXPECT_EQ( d.value(), "d: got const b" );
}

struct human {};
struct wolf {};
struct star {};

template < typename T > struct dispatched : public comma::dispatch::dispatched< dispatched< T > >
                                          , public T
{
};

struct hunter : public dispatch::handler_of< dispatched< human > >
              , public dispatch::handler_of< dispatched< wolf > >
              , public dispatch::handler_of< dispatched< star > >
{
    void handle( dispatched< human >& h ) { std::cerr << "    said hi to a human" << std::endl; }
    void handle( dispatched< wolf >& w ) { std::cerr << "    shot a wolf" << std::endl; }
    void handle( dispatched< star >& s ) { std::cerr << "    saw a star" << std::endl; }
};

struct soldier : public dispatch::handler_of< dispatched< human > >
               , public dispatch::handler_of< dispatched< wolf > >
               , public dispatch::handler_of< dispatched< star > >
{
    void handle( dispatched< human >& h ) { std::cerr << "    shot a human" << std::endl; }
    void handle( dispatched< wolf >& w ) { std::cerr << "    said hi to a wolf" << std::endl; }
    void handle( dispatched< star >& s ) { std::cerr << "    saw aliens landing" << std::endl; }
};

TEST( dispatch, DISABLED_non_invasive_example ) // enable to see it working
{
    comma::dispatch::handler* h = new hunter;
    comma::dispatch::handler* s = new soldier;
    comma::dispatch::dispatched_base* dh = new dispatched< human >;
    comma::dispatch::dispatched_base* dw = new dispatched< wolf >;
    comma::dispatch::dispatched_base* ds = new dispatched< star >;
    std::cerr  << std::endl << "hunter:" << std::endl;
    dh->dispatch_to( h );
    dw->dispatch_to( h );
    ds->dispatch_to( h );
    std::cerr  << std::endl << "soldier:" << std::endl;
    dh->dispatch_to( s );
    dw->dispatch_to( s );
    ds->dispatch_to( s );
}

struct fish : public comma::dispatch::dispatched< fish > {};

struct butterfly : public comma::dispatch::dispatched< butterfly > {};

struct fisherman : public dispatch::handler_of< fish >
                 , public dispatch::handler_of< butterfly >
{
    void handle( fish& f ) { std::cerr << "    caught a fish" << std::endl; }
    void handle( butterfly& f ) { std::cerr << "    used butterfly as a bait" << std::endl; }
};

struct scientist : public dispatch::handler_of< fish >
                 , public dispatch::handler_of< butterfly >
{
    void handle( fish& f ) { std::cerr << "    does not care about fish" << std::endl; }
    void handle( butterfly& f ) { std::cerr << "    pinned butterfly in his collection" << std::endl; }
};

TEST( dispatch, DISABLED_invasive_example ) // enable to see it working
{
    comma::dispatch::handler* f = new fisherman;
    comma::dispatch::handler* s = new scientist;
    comma::dispatch::dispatched_base* df = new fish;
    comma::dispatch::dispatched_base* db = new butterfly;
    std::cerr  << std::endl << "fisherman:" << std::endl;
    df->dispatch_to( f );
    db->dispatch_to( f );
    std::cerr  << std::endl << "scientist:" << std::endl;
    df->dispatch_to( s );
    db->dispatch_to( s );
}

} } // namespace comma { namespace test {

int main( int argc, char* argv[] )
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
