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
// 3. All advertising materials mentioning features or use of this software
//    must display the following acknowledgement:
//    This product includes software developed by the The University of Sydney.
// 4. Neither the name of the The University of Sydney nor the
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
#include <limits>
#include "boost/date_time/posix_time/posix_time.hpp"
#include <comma/csv/format.h>
#include <comma/csv/options.h>
#include <comma/csv/impl/unstructured.h>

TEST( csv, format )
{
    {
        try { comma::csv::format f( "%" ); EXPECT_TRUE( false ); } catch ( ... ) {}
        try { comma::csv::format f( "blah" ); EXPECT_TRUE( false ); } catch ( ... ) {}
    }
    {
        comma::csv::format f( "%uw" );
        EXPECT_EQ( f.bin_to_csv( f.csv_to_bin( "1234" ) ), "1234" );
    }
    {
        comma::csv::format f( "%w" );
        EXPECT_EQ( f.bin_to_csv( f.csv_to_bin( "-1234" ) ), "-1234" );
    }
    {
        comma::csv::format f( "%ui" );
        EXPECT_EQ( f.bin_to_csv( f.csv_to_bin( "1234" ) ), "1234" );
    }
    {
        comma::csv::format f( "%i" );
        EXPECT_EQ( f.bin_to_csv( f.csv_to_bin( "-1234" ) ), "-1234" );
    }
    {
        comma::csv::format f( "%ul" );
        EXPECT_EQ( f.bin_to_csv( f.csv_to_bin( "1234" ) ), "1234" );
    }
    {
        comma::csv::format f( "%l" );
        EXPECT_EQ( f.bin_to_csv( f.csv_to_bin( "-1234" ) ), "-1234" );
    }
    {
        comma::csv::format f( "%c" );
        EXPECT_EQ( f.bin_to_csv( f.csv_to_bin( "c" ) ), "c" );
    }
    {
        comma::csv::format f( "%f" );
        EXPECT_EQ( f.bin_to_csv( f.csv_to_bin( "1234.56" ) ), "1234.56" ); // floats have just 6-digit precision
    }
    {
        comma::csv::format f( "%d" );
        EXPECT_EQ( f.bin_to_csv( f.csv_to_bin( "1234.123456789012" ) ), "1234.123456789012" );
    }
    {
        comma::csv::format f( "%uw%ui%f" );
        EXPECT_EQ( f.bin_to_csv( f.csv_to_bin( "1234,5678,90.1234" ) ), "1234,5678,90.1234" );
    }
    {
        comma::csv::format f( "%t" );
        EXPECT_EQ( f.bin_to_csv( f.csv_to_bin( "20100621T182601" ) ), "20100621T182601" );
    }
    {
        comma::csv::format f( "%t" );
        EXPECT_EQ( f.bin_to_csv( f.csv_to_bin( "20100621T182601.0123" ) ), "20100621T182601.012300" );
    }
    {
        comma::csv::format f( "%t" );
        EXPECT_EQ( f.bin_to_csv( f.csv_to_bin( "20100621T182601.123" ) ), "20100621T182601.123000" );
    }
    {
        comma::csv::format f( "%3ui" );
        EXPECT_EQ( f.bin_to_csv( f.csv_to_bin( "1,2,3" ) ), "1,2,3" );
    }
    {
        comma::csv::format f( "%ui%2w%3d" );
        EXPECT_EQ( f.bin_to_csv( f.csv_to_bin( "0,-1,-2,1.123,2.345,3.678" ) ), "0,-1,-2,1.123,2.345,3.678" );
    }
    {
        comma::csv::format f( "%ui%2w%3d" );
        EXPECT_EQ( f.index( 0 ).first, 0u );
        EXPECT_EQ( f.index( 0 ).second, 0u );
        EXPECT_EQ( f.index( 1 ).first, 1u );
        EXPECT_EQ( f.index( 1 ).second, 0u );
        EXPECT_EQ( f.index( 2 ).first, 1u );
        EXPECT_EQ( f.index( 2 ).second, 1u );
        EXPECT_EQ( f.index( 3 ).first, 2u );
        EXPECT_EQ( f.index( 3 ).second, 0u );
        EXPECT_EQ( f.index( 4 ).first, 2u );
        EXPECT_EQ( f.index( 4 ).second, 1u );
        EXPECT_EQ( f.index( 5 ).first, 2u );
        EXPECT_EQ( f.index( 5 ).second, 2u );
    }    
    {
        try { comma::csv::format f( "%s" ); EXPECT_TRUE( false ); } catch ( ... ) {}
    }
    {
        comma::csv::format f( "%s[4]%2s[8]%10ui" );
        EXPECT_EQ( f.elements()[0].count, 1u );
        EXPECT_EQ( f.elements()[0].offset, 0u );
        EXPECT_EQ( f.elements()[0].size, 4u );
        EXPECT_EQ( f.elements()[1].count, 2u );
        EXPECT_EQ( f.elements()[1].offset, 4u );
        EXPECT_EQ( f.elements()[1].size, 8u );
        EXPECT_EQ( f.elements()[2].count, 10u );
        EXPECT_EQ( f.elements()[2].offset, 20u );
        EXPECT_EQ( f.elements()[2].size, 4u );
    }
    {
        comma::csv::format f( "%ui%2s[4]%3d" );
        EXPECT_EQ( f.index( 0 ).first, 0u );
        EXPECT_EQ( f.index( 0 ).second, 0u );
        EXPECT_EQ( f.index( 1 ).first, 1u );
        EXPECT_EQ( f.index( 1 ).second, 0u );
        EXPECT_EQ( f.index( 2 ).first, 1u );
        EXPECT_EQ( f.index( 2 ).second, 1u );
        EXPECT_EQ( f.index( 3 ).first, 2u );
        EXPECT_EQ( f.index( 3 ).second, 0u );
        EXPECT_EQ( f.index( 4 ).first, 2u );
        EXPECT_EQ( f.index( 4 ).second, 1u );
        EXPECT_EQ( f.index( 5 ).first, 2u );
        EXPECT_EQ( f.index( 5 ).second, 2u );
    }    
}

TEST( csv, format_add )
{
    {
        comma::csv::format f;
        comma::csv::format d;
        EXPECT_EQ( ( f += d ).string(), "" );
    }
    {
        comma::csv::format f( "i,f" );
        comma::csv::format d;
        EXPECT_EQ( ( f += d ).string(), "i,f" );
    }
    {
        comma::csv::format f;
        comma::csv::format d( "ui,d" );
        EXPECT_EQ( ( f += d ).string(), "ui,d" );
    }
    {
        comma::csv::format f( "f" );
        comma::csv::format d( "d" );
        EXPECT_EQ( ( f += d ).string(), "f,d" );
    }
    {
        comma::csv::format f( "i,f" );
        comma::csv::format d( "ui,d" );
        EXPECT_EQ( ( f += d ).string(), "i,f,ui,d" );
    }
}

TEST( csv, format_floating_point )
{
    {
        comma::csv::format f( "%f" );
        EXPECT_EQ( f.bin_to_csv( f.csv_to_bin( "1234.56" ) ), "1234.56" ); // floats have just 6-digit precision
    }
// todo more tests
}

//TEST( csv, format_nan )
//{
//	double nan = std::numeric_limits< double >::quiet_NaN();
//	double d = nan;
//	EXPECT_TRUE( std::isnan( nan ) );
//	EXPECT_TRUE( std::isnan( d ) );
//	EXPECT_TRUE( !std::isnan( 5 ) );
//	EXPECT_TRUE( 5 != nan );
//	std::cerr << "double nan: ";
//	std::cerr << nan << ": ";
//	const unsigned char* p = reinterpret_cast< const unsigned char* >( &nan );
//	for( unsigned int i = 0; i < sizeof( double ); ++i, ++p )
//	{
//		std::cerr << static_cast< unsigned int >( *p ) << " ";
//	}
//	std::cerr << std::endl;
//}

struct nested_struct
{
    comma::int32 x;
    comma::int32 y;
};

struct simple_struct
{
    comma::int32 a;
    double b;
    char c;
    std::string s;
    boost::posix_time::ptime t;
    nested_struct nested;
};

struct optionals
{
    boost::optional< double > a;
    boost::optional< nested_struct > b;
};

namespace comma { namespace visiting {

template <> struct traits< nested_struct >
{
    template < typename Key, class Visitor > static void visit( const Key&, const nested_struct& p, Visitor& v )
    {
        v.apply( "x", p.x );
        v.apply( "y", p.y );
    }
};

template <> struct traits< simple_struct >
{
    template < typename Key, class Visitor > static void visit( const Key&, const simple_struct& p, Visitor& v )
    {
        v.apply( "a", p.a );
        v.apply( "b", p.b );
        v.apply( "c", p.c );
        v.apply( "s", p.s );
        v.apply( "t", p.t );
        v.apply( "nested", p.nested );
    }
};

template <> struct traits< optionals >
{
    template < typename Key, class Visitor > static void visit( const Key&, const optionals& p, Visitor& v )
    {
        v.apply( "a", p.a );
        v.apply( "b", p.b );
    }
};

} } // namespace comma { namespace visiting {

TEST( csv, format_with_fields )
{
    EXPECT_EQ( comma::csv::format::value< simple_struct >(), "i,d,b,s,t,i,i" );
    EXPECT_EQ( comma::csv::format::value< simple_struct >( "a,b,c", true ), "i,d,b" );
    EXPECT_EQ( comma::csv::format::value< simple_struct >( "a,s,t", true ), "i,s,t" );
    EXPECT_EQ( comma::csv::format::value< simple_struct >( "nested/x,nested/y", true ), "i,i" );
    EXPECT_EQ( comma::csv::format::value< simple_struct >( "nested", true ), "i,i" );
    EXPECT_EQ( comma::csv::format::value< simple_struct >( "a,b,c", false ), "i,d,b" );
    EXPECT_EQ( comma::csv::format::value< simple_struct >( "a,s,t", false ), "i,s,t" );
    EXPECT_EQ( comma::csv::format::value< simple_struct >( "x,y", false ), "i,i" );
}

TEST( csv, optional_format )
{
    EXPECT_EQ( comma::csv::format::value< optionals >(), "d,i,i" );
}

TEST( csv, unstructured )
{
    EXPECT_EQ( "d,d,d,d", comma::csv::impl::unstructured::guess_format( "1,2,3,4" ).string() );
    EXPECT_EQ( "d,d,t,s[1024]", comma::csv::impl::unstructured::guess_format( "1,2.1,20121212T000000,blah" ).string() );
    comma::csv::options csv;
    csv.fields = "a,,,b,,,c";
    csv.delimiter = ',';
    EXPECT_EQ( "d,s[1024],s[1024],s[1024],s[1024],s[1024],t", comma::csv::impl::unstructured::guess_format( "1,,,blah,,,20121212T000000" ).string() );
    EXPECT_EQ( 1, comma::csv::impl::unstructured::make( csv, "1,,,blah,,,20121212T000000" ).first.doubles.size() );
    EXPECT_EQ( 1, comma::csv::impl::unstructured::make( csv, "1,,,blah,,,20121212T000000" ).first.strings.size() );
    EXPECT_EQ( 1, comma::csv::impl::unstructured::make( csv, "1,,,blah,,,20121212T000000" ).first.time.size() );
    EXPECT_EQ( "d[0],,,s[0],,,t[0]", comma::csv::impl::unstructured::make( csv, "1,,,blah,,,20121212T000000" ).second.fields );
    // todo: certainly more tests
}
