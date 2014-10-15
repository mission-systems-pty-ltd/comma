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


#ifndef WIN32
#include <stdlib.h>
#endif
#include <math.h>
#include <iostream>
#include <gtest/gtest.h>
#include <comma/packed/packed.h>

struct test_packed_struct_t : public comma::packed::packed_struct< test_packed_struct_t, 16 >
{
    comma::packed::string< 4 > hello;
    comma::packed::string< 5 > world;
    comma::packed::net_uint16 int16;
    comma::packed::net_uint32 int32;
    comma::packed::byte byte;
};

TEST( packed_struct, test_packed_struct )
{
    test_packed_struct_t s;
    EXPECT_EQ( s.hello == comma::packed::string< 4 >::default_value(), true );
    EXPECT_EQ( s.world == comma::packed::string< 5 >::default_value(), true );
    EXPECT_EQ( s.hello(), comma::packed::string< 4 >::default_value() );
    EXPECT_EQ( s.world(), comma::packed::string< 5 >::default_value() );
    EXPECT_EQ( s.world != std::string( "blah" ), true );
    s.world = "WORLD";
    EXPECT_EQ( s.world == std::string( "WORLD" ), true );
    EXPECT_EQ( s.int16 == 0, true );
    EXPECT_EQ( s.int32 == 0, true );
    s.int16 = 1;
    s.int32 = 2;
    s.byte = 3;
    EXPECT_EQ( s.int16 == 1, true );
    EXPECT_EQ( s.int32 == 2, true );
    EXPECT_EQ( s.byte == 3, true );
    EXPECT_EQ( s.int16(), 1 );
    EXPECT_EQ( s.int32(), 2 );
    EXPECT_EQ( s.byte(), 3 );
}

template < typename T >
void test_packed_int( int value )
{
    T t;
    EXPECT_EQ( t == 0, true );
    t = value;
    EXPECT_EQ( t == value, true );
    EXPECT_EQ( t(), value );
}

TEST( test_packed_struct_test, test_little_endian )
{
    test_packed_int< comma::packed::uint16 >( 1234 );
    test_packed_int< comma::packed::uint24 >( 1234 );
    test_packed_int< comma::packed::uint32 >( 1234 );
    test_packed_int< comma::packed::int16 >( 1234 );
    test_packed_int< comma::packed::int24 >( 1234 );
    test_packed_int< comma::packed::int32 >( 1234 );
    test_packed_int< comma::packed::int16 >( -1234 );
    test_packed_int< comma::packed::int24 >( -1234 );
    test_packed_int< comma::packed::int32 >( -1234 );
}

TEST( test_packed_struct_test, test_big_endian )
{
    test_packed_int< comma::packed::net_uint16 >( 1234 );
    test_packed_int< comma::packed::net_uint32 >( 1234 );
}

static void test_int24_byte_order( int value, char byte0, char byte1, char byte2 )
{
    comma::packed::int24 a;
    a = value;
    EXPECT_EQ( ( 0xff & a.data()[0] ), ( 0xff & byte0 ) );
    EXPECT_EQ( ( 0xff & a.data()[1] ), ( 0xff & byte1 ) );
    EXPECT_EQ( ( 0xff & a.data()[2] ), ( 0xff & byte2 ) );
}

TEST( test_packed_struct_test, test_int24_byte_order )
{
    test_int24_byte_order( 0, 0x00, 0x00, 0x00 );
    test_int24_byte_order( 1, 0x01, 0x00, 0x00 );
    test_int24_byte_order( 2, 0x02, 0x00, 0x00 );
    test_int24_byte_order( 32767, 0xff, 0x7f, 0x00 );
    test_int24_byte_order( 8388607, 0xff, 0xff, 0x7f );
    test_int24_byte_order( -1, 0xff, 0xff, 0xff );
    test_int24_byte_order( -2, 0xfe, 0xff, 0xff );
    test_int24_byte_order( -32767, 0x01, 0x80, 0xff );
    test_int24_byte_order( -32768, 0x00, 0x80, 0xff );
    test_int24_byte_order( -8388607, 0x01, 0x00, 0x80 );
    test_int24_byte_order( -8388608, 0x00, 0x00, 0x80 );
}

static boost::array< std::string, 16 > hex_digits_u = { { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F" } };
static boost::array< std::string, 16 > hex_digits_l = { { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "a", "b", "c", "d", "e", "f" } };

template < typename T, unsigned int Size >
void test_ascii_hex_default_value()
{
    unsigned int expected_default_value = 0;
    comma::packed::ascii_hex< T, Size > a;
    EXPECT_EQ( expected_default_value, a.default_value() );
}

TEST( test_packed_ascii_hex, test_default_value )
{
    test_ascii_hex_default_value< comma::uint16, 1 >();
    test_ascii_hex_default_value< comma::uint16, 2 >();
    test_ascii_hex_default_value< comma::uint16, 3 >();
}

template < typename T, unsigned int Size >
void test_ascii_hex_default_padding( std::string hex_with_padding, T expected_decimal )
{
    comma::packed::ascii_hex< T, Size > a;
    EXPECT_EQ(  expected_decimal, a.unpack( hex_with_padding.c_str() ) );
}

TEST( test_packed_ascii_hex, test_default_padding )
{
    test_ascii_hex_default_padding< comma::uint16, 2 >( " f", 15 );
    test_ascii_hex_default_padding< comma::uint16, 3 >( "  f", 15 );
    test_ascii_hex_default_padding< comma::uint16, 3 >( " ff", 255 );
    test_ascii_hex_default_padding< comma::uint16, 3 >( "fff", 4095 );
}

template < typename T, unsigned int Size, char Padding >
void test_ascii_hex_padding( std::string hex_with_padding, T expected_decimal )
{
    comma::packed::ascii_hex< T, Size, Padding > a;
    EXPECT_EQ( expected_decimal, a.unpack( hex_with_padding.c_str() ) );
}

TEST( test_packed_ascii_hex, test_padding )
{
    test_ascii_hex_padding< comma::uint16, 2, '0' >( "0f", 15 );
    test_ascii_hex_padding< comma::uint16, 3, '0' >( "00f", 15 );
    test_ascii_hex_padding< comma::uint16, 3, '0' >( "0ff", 255 );
    test_ascii_hex_padding< comma::uint16, 3, '0' >( "fff", 4095 );
    test_ascii_hex_padding< comma::uint16, 2, '*' >( "*f", 15 );
}

template < typename T, unsigned int Size >
void test_ascii_hex_unpack_size( const std::string& hex, T expected_decimal )
{
    comma::packed::ascii_hex< T, Size > a;
    EXPECT_EQ( expected_decimal, a.unpack( hex.c_str() ) );
}

TEST( test_packed_ascii_hex, test_unpack_size )
{
    test_ascii_hex_unpack_size< comma::uint16, 1 >( "f", 15 );
    test_ascii_hex_unpack_size< comma::uint16, 2 >( "f", 15 );
    test_ascii_hex_unpack_size< comma::uint16, 2 >( "ff", 255 );
    test_ascii_hex_unpack_size< comma::uint16, 3 >( "f", 15 );
    test_ascii_hex_unpack_size< comma::uint16, 3 >( "ff", 255 );
    test_ascii_hex_unpack_size< comma::uint16, 3 >( "fff", 4095 );
}

template < typename T >
void test_ascii_hex_unpack_values_size1( const boost::array< std::string, 16 >& hex_digits )
{
    comma::packed::ascii_hex< T, 1 > a;
    for( unsigned int i = 0; i < hex_digits.size(); ++i ) 
    {
        std::string hex = hex_digits[i];
        T expected_decimal = i;
        EXPECT_EQ( expected_decimal, a.unpack( hex.c_str() ) );
    }
}

TEST( test_packed_ascii_hex, test_unpack_values_size_1_uppercase )
{    
    test_ascii_hex_unpack_values_size1< comma::uint16 >( hex_digits_u );
}

TEST( test_packed_ascii_hex, test_unpack_values_size_1_lowercase )
{   
    test_ascii_hex_unpack_values_size1< comma::uint16 >( hex_digits_l );
}

template < typename T >
void test_ascii_hex_unpack_values_size2( const boost::array< std::string, 16 >& hex_digits )
{
    comma::packed::ascii_hex< T, 2 > a;
    for( unsigned int i = 0; i < hex_digits.size(); ++i ) 
    {
        for( unsigned int j = 0; j < hex_digits.size(); ++j )
        {
            std::string hex = hex_digits[i] + hex_digits[j];
            T expected_decimal = i*16 + j;
            EXPECT_EQ( expected_decimal, a.unpack( hex.c_str() ) );
        }
    }    
}

TEST( test_packed_ascii_hex, test_unpack_values_size_2_uppercase )
{    
    test_ascii_hex_unpack_values_size2< comma::uint16 >( hex_digits_u );
}

TEST( test_packed_ascii_hex, test_unpack_values_size_2_lowercase )
{   
    test_ascii_hex_unpack_values_size2< comma::uint16 >( hex_digits_l );
}

template < typename T >
void test_ascii_hex_unpack_values_size3( const boost::array< std::string, 16 >& hex_digits )
{
    comma::packed::ascii_hex< T, 3 > a;
    for( unsigned int i = 0; i < hex_digits.size(); ++i ) 
    {
        for( unsigned int j = 0; j < hex_digits.size(); ++j )
        {
            for( unsigned int k = 0; k < hex_digits.size(); ++k )
            {
                std::string hex = hex_digits[i] + hex_digits[j] + hex_digits[k];
                T expected_decimal = i*16*16 + j*16 + k;
                EXPECT_EQ( expected_decimal, a.unpack( hex.c_str() ) );
            }
        }
    }    
}

TEST( test_packed_ascii_hex, test_unpack_values_size_3_uppercase )
{    
    test_ascii_hex_unpack_values_size3< comma::uint16 >( hex_digits_u );
}

TEST( test_packed_ascii_hex, test_unpack_values_size_3_lowercase )
{   
    test_ascii_hex_unpack_values_size3< comma::uint16 >( hex_digits_l );
}

template < typename T >
void test_ascii_hex_pack_values_size1( const boost::array< std::string, 16 >& hex_digits )
{
    comma::packed::ascii_hex< T, 1 > a;
    char buf[] = "X";
    for( unsigned int i = 0; i < hex_digits.size(); ++i ) 
    {
        const T decimal = i;
        a.pack( buf, decimal );
        EXPECT_EQ( hex_digits[i], std::string( buf, 1 ) );
    }
}

TEST( test_packed_ascii_hex, test_pack_values_size_1 )
{
    test_ascii_hex_pack_values_size1< comma::uint16 >( hex_digits_l );
}

template < typename T >
void test_ascii_hex_pack_values_size2( const boost::array< std::string, 16 >& hex_digits )
{
    comma::packed::ascii_hex< T, 2, '0' > a;
    char buf[] = "XX";
    for( unsigned int i = 0; i < hex_digits.size(); ++i ) 
    {
        for( unsigned int j = 0; j < hex_digits.size(); ++j )
        {
            const T decimal = i*16 + j;
            a.pack( buf, decimal );
            EXPECT_EQ( hex_digits[i] + hex_digits[j], std::string( buf, 2 ) );
        }
    }    
}

TEST( test_packed_ascii_hex, test_pack_values_size_2 )
{
    test_ascii_hex_pack_values_size2< comma::uint16 >( hex_digits_l );
}

template < typename T >
void test_ascii_hex_pack_values_size3( const boost::array< std::string, 16 >& hex_digits )
{
    comma::packed::ascii_hex< T, 3, '0' > a;
    char buf[] = "XXX";
    for( unsigned int i = 0; i < hex_digits.size(); ++i ) 
    {
        for( unsigned int j = 0; j < hex_digits.size(); ++j )
        {
            for( unsigned int k = 0; k < hex_digits.size(); ++k )
            {
                const T decimal = i*16*16 + j*16 + k;
                a.pack( buf, decimal );
                EXPECT_EQ( hex_digits[i] + hex_digits[j] + hex_digits[k], std::string( buf, 3 ) );
            }
        }
    }    
}

TEST( test_packed_ascii_hex, test_pack_values_size_3 )
{
    test_ascii_hex_pack_values_size3< comma::uint16 >( hex_digits_l );
}

TEST( test_packed_ascii_hex, test_pack_default_padding )
{
    comma::packed::ascii_hex< comma::uint16, 2, ' ' > a;
    char hex2[] = "XX";
    a.pack( hex2, 0 ); EXPECT_EQ( " 0", std::string( hex2, 2 ) );
    a.pack( hex2, 15 ); EXPECT_EQ( " f", std::string( hex2, 2 ) );
    
    comma::packed::ascii_hex< comma::uint16, 3 > b;
    char hex3[] = "XXX";
    b.pack( hex3, 0 ); EXPECT_EQ( "  0", std::string( hex3, 3 ) );
    b.pack( hex3, 15 ); EXPECT_EQ( "  f", std::string( hex3, 3 ) );
    b.pack( hex3, 255 ); EXPECT_EQ( " ff", std::string( hex3, 3 ) );    
}    

TEST( test_packed_ascii_hex, test_pack_padding )
{
    comma::packed::ascii_hex< comma::uint16, 3, '0' > c;
    char hex3[] = "XXX";
    c.pack( hex3, 0 ); EXPECT_EQ( "000", std::string( hex3, 3 ) );
    c.pack( hex3, 15 ); EXPECT_EQ( "00f", std::string( hex3, 3 ) );
    c.pack( hex3, 255 ); EXPECT_EQ( "0ff", std::string( hex3, 3 ) );
    c.pack( hex3, 4095 ); EXPECT_EQ( "fff", std::string( hex3, 3 ) );
}

struct ascii_hex_struct : public comma::packed::packed_struct< ascii_hex_struct, 9 >
{
    comma::packed::const_byte< ' ' > p1;
    comma::packed::ascii_hex< comma::uint32, 7 > value;
    comma::packed::const_byte< ' ' > p2;
};

TEST( test_packed_ascii_hex, test_with_cast )
{
    std::string hex1 = "       0 ";
    EXPECT_EQ( 0, reinterpret_cast< const ascii_hex_struct* >( &hex1[0] )->value() );
    
    std::string hex2 = " 1234567 ";
    EXPECT_EQ( 19088743, reinterpret_cast< const ascii_hex_struct* >( &hex2[0] )->value() );

    std::string hex3 = " abcdef0 ";
    EXPECT_EQ( 180150000, reinterpret_cast< const ascii_hex_struct* >( &hex3[0] )->value() );
}

TEST( test_packed_ascii_hex, test_throw_unexpected_hexadecimal_digit )
{
    comma::packed::ascii_hex< comma::uint16, 1 > a;
    ASSERT_THROW( a.unpack( "g" ), comma::exception );
}

int main( int argc, char *argv[] )
{
    ::testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS();
}
