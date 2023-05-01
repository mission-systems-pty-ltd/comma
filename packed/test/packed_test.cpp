// Copyright (c) 2011 The University of Sydney

#ifndef WIN32
#include <stdlib.h>
#endif
#include <math.h>
#include <iostream>
#include <gtest/gtest.h>
#include <boost/array.hpp>
#include "../../packed/packed.h"
#include "../../packed/traits.h"
#include "../../math/compare.h"
#include "../../packed/bits.h"
#include "../../base/types.h"
#include "../../visiting/traits.h"
#include "../../csv/stream.h"
#include "../../csv/options.h"

struct test_packed_struct_t : public comma::packed::packed_struct< test_packed_struct_t, 16 >
{
    comma::packed::string< 4 > hello;
    comma::packed::string< 5 > world;
    comma::packed::big_endian::uint16 int16;
    comma::packed::big_endian::uint32 int32;
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

template < typename T > static void test_packed_int( comma::int64 value )
{
    T t;
    EXPECT_EQ( true, t == 0 );
    t = value;
    EXPECT_EQ( true, t == value );
    std::cerr << "-------------------------------------------------" << std::endl;
    std::cerr << "--> a: value: " << value << " t: " << t() << std::endl;
    EXPECT_EQ( value, t() );
    std::cerr << "-------------------------------------------------" << std::endl;
}

template < typename T > static void test_packed_uint( comma::uint64 value )
{
    T t;
    EXPECT_EQ( true, t == 0 );
    t = value;
    EXPECT_EQ( true, t == value );
    std::cerr << "-------------------------------------------------" << std::endl;
    std::cerr << "--> b: value: " << value << " t: " << t() << std::endl;
    EXPECT_EQ( value, t() );
    std::cerr << "-------------------------------------------------" << std::endl;
}

TEST( test_packed_struct_test, test_little_endian )
{
    test_packed_uint< comma::packed::little_endian::uint16 >( 1231 );
    test_packed_uint< comma::packed::little_endian::uint16 >( 65535 );
    test_packed_uint< comma::packed::little_endian::uint24 >( 1232 );
    test_packed_uint< comma::packed::little_endian::uint24 >( 16777215 );
    test_packed_uint< comma::packed::little_endian::uint32 >( 1233 );
    test_packed_uint< comma::packed::little_endian::uint32 >( 4294967295 );
    test_packed_uint< comma::packed::little_endian::uint64 >( 4321 );
    test_packed_uint< comma::packed::little_endian::uint64 >( comma::uint64( std::numeric_limits< comma::uint64 >::max() ) );
    test_packed_uint< comma::packed::little_endian::uint64 >( comma::uint64( 0x1BCDEF1213141500ULL ) );

    test_packed_int< comma::packed::little_endian::int16 >( 1234 );
    test_packed_int< comma::packed::little_endian::int16 >( 256 * 128 - 1 );
    test_packed_int< comma::packed::little_endian::int16 >( 0 );
    test_packed_int< comma::packed::little_endian::int16 >( -1 );
    test_packed_int< comma::packed::little_endian::int16 >( -2 );
    test_packed_int< comma::packed::little_endian::int16 >( -256 * 128 + 1 );
    //for( comma::int16 i = 256 * 128 - 1; i > 0; --i ) { test_packed_uint< comma::packed::little_endian::int16 >( i ); }
    //for( comma::int16 i = 256 * 128 - 1; i > 0; --i ) { test_packed_int< comma::packed::little_endian::int16 >( -i ); }
    test_packed_int< comma::packed::little_endian::int24 >( 1235 );
    test_packed_int< comma::packed::little_endian::int24 >( 8388607 );
    test_packed_int< comma::packed::little_endian::int32 >( 8388607 );
    test_packed_int< comma::packed::little_endian::int32 >( 1236 );
    test_packed_int< comma::packed::little_endian::int16 >( -1231 );
    test_packed_int< comma::packed::little_endian::int24 >( -1 );
    test_packed_int< comma::packed::little_endian::int24 >( -2 );
    test_packed_int< comma::packed::little_endian::int24 >( -256 );
    test_packed_int< comma::packed::little_endian::int24 >( -1232 );
    //for( unsigned int i = 0; i < 8388608; ++i ) { test_packed_int< comma::packed::little_endian::int24 >( -i ); }
    test_packed_int< comma::packed::little_endian::int24 >( -1000000 );
    test_packed_int< comma::packed::little_endian::int24 >( -8388608 );
    test_packed_int< comma::packed::little_endian::int32 >( -1233 );
    test_packed_int< comma::packed::little_endian::int64 >( -4321 );
    test_packed_int< comma::packed::little_endian::int64 >( comma::int64( std::numeric_limits< comma::int64 >::min() ) );
}

TEST( test_packed_struct_test, test_big_endian )
{
    test_packed_int< comma::packed::big_endian::uint16 >( 1234 );
    test_packed_int< comma::packed::big_endian::uint16 >( 65535 );
    test_packed_uint< comma::packed::big_endian::uint24 >( 1232 );
    test_packed_uint< comma::packed::big_endian::uint24 >( 16777215 );
    test_packed_int< comma::packed::big_endian::uint32 >( 1234 );
    test_packed_int< comma::packed::big_endian::uint32 >( 4294967295 );
    test_packed_int< comma::packed::big_endian::int16 >( 1234 );
    test_packed_int< comma::packed::big_endian::int32 >( 1234 );
    test_packed_int< comma::packed::big_endian::int16 >( -1234 );
    test_packed_int< comma::packed::big_endian::int32 >( -1234 );
    test_packed_int< comma::packed::big_endian::int24 >( -1 );
    test_packed_int< comma::packed::big_endian::int24 >( -2 );
    test_packed_int< comma::packed::big_endian::int24 >( -1232 );
    test_packed_int< comma::packed::big_endian::int24 >( -8388607 );
    test_packed_int< comma::packed::big_endian::int24 >( -8388608 );
}

template< typename T >
static void test_int64_byte_order( comma::int64 value, char byte0, char byte1, char byte2, char byte3, char byte4, char byte5, char byte6, char byte7 )
{
    T a;
    a = value;
    EXPECT_EQ( ( 0xff & byte0 ), ( 0xff & a.data()[0] ) );
    EXPECT_EQ( ( 0xff & byte1 ), ( 0xff & a.data()[1] ) );
    EXPECT_EQ( ( 0xff & byte2 ), ( 0xff & a.data()[2] ) );
    EXPECT_EQ( ( 0xff & byte3 ), ( 0xff & a.data()[3] ) );
    EXPECT_EQ( ( 0xff & byte4 ), ( 0xff & a.data()[4] ) );
    EXPECT_EQ( ( 0xff & byte5 ), ( 0xff & a.data()[5] ) );
    EXPECT_EQ( ( 0xff & byte6 ), ( 0xff & a.data()[6] ) );
    EXPECT_EQ( ( 0xff & byte7 ), ( 0xff & a.data()[7] ) );
}

TEST( test_packed_struct_test, test_int64_byte_order )
{
    comma::int64 i = 0xFBCDEF1213141500LL;
    test_int64_byte_order< comma::packed::little_endian::uint64 >( i, 0x00, 0x15, 0x14, 0x13, 0x12, 0xEF, 0xCD, 0xFB );
}

template< typename T >
static void test_uint64_byte_order( comma::uint64 value, char byte0, char byte1, char byte2, char byte3, char byte4, char byte5, char byte6, char byte7 )
{
    T a;
    a = value;
    EXPECT_EQ( ( 0xff & byte0 ), ( 0xff & a.data()[0] ) );
    EXPECT_EQ( ( 0xff & byte1 ), ( 0xff & a.data()[1] ) );
    EXPECT_EQ( ( 0xff & byte2 ), ( 0xff & a.data()[2] ) );
    EXPECT_EQ( ( 0xff & byte3 ), ( 0xff & a.data()[3] ) );
    EXPECT_EQ( ( 0xff & byte4 ), ( 0xff & a.data()[4] ) );
    EXPECT_EQ( ( 0xff & byte5 ), ( 0xff & a.data()[5] ) );
    EXPECT_EQ( ( 0xff & byte6 ), ( 0xff & a.data()[6] ) );
    EXPECT_EQ( ( 0xff & byte7 ), ( 0xff & a.data()[7] ) );
}

TEST( test_packed_struct_test, test_uint64_byte_order )
{
    comma::uint64 i = 0xABCDEF1213141500ULL;
    test_uint64_byte_order< comma::packed::little_endian::uint64 >( i, 0x00, 0x15, 0x14, 0x13, 0x12, 0xEF, 0xCD, 0xAB );
}

static void test_int24_byte_order( int value, char byte0, char byte1, char byte2 )
{
    comma::packed::little_endian::int24 a;
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

struct test_packed_struct_floats_t : public comma::packed::packed_struct< test_packed_struct_floats_t, 24 >
{
    comma::packed::little_endian::float32 f32;
    comma::packed::little_endian::float64 f64;
    comma::packed::big_endian::float32 nf32;
    comma::packed::big_endian::float64 nf64;
};

TEST( packed_struct, test_packed_struct_floats )
{
    test_packed_struct_floats_t s;
    EXPECT_FLOAT_EQ( true, s.f32 == 0 );
    EXPECT_DOUBLE_EQ( true, s.f64 == 0 );
    s.f32 = 1.2345;
    s.f64 = 1.23456789;
    EXPECT_FLOAT_EQ( 1.2345, s.f32() );
    EXPECT_DOUBLE_EQ( 1.23456789, s.f64() );
}

TEST( packed_struct, test_packed_struct_big_endian_floats )
{
    test_packed_struct_floats_t s;
    EXPECT_FLOAT_EQ( true, s.nf32 == 0 );
    EXPECT_DOUBLE_EQ( true, s.nf64 == 0 );
    s.nf32 = 1.2345;
    s.nf64 = 1.23456789;
    EXPECT_FLOAT_EQ( 1.2345, s.nf32() );
    EXPECT_DOUBLE_EQ( 1.23456789, s.nf64() );
}

TEST( test_packed_struct_test, test_little_endian_floats )
{
    comma::packed::little_endian::float32 a;
    EXPECT_FLOAT_EQ( 0, a() );
    a = 1.2345;
    EXPECT_FLOAT_EQ( 1.2345, a() );

    comma::packed::little_endian::float64 b;
    EXPECT_DOUBLE_EQ( 0, b() );
    b = 1.23456789;
    EXPECT_DOUBLE_EQ( 1.23456789, b() );
}

TEST( test_packed_struct_test, test_big_endian_floats )
{
    comma::packed::big_endian::float32 a;
    EXPECT_FLOAT_EQ( 0, a() );
    a = 1.2345;
    EXPECT_FLOAT_EQ( 1.2345, a() );

    comma::packed::big_endian::float64 b;
    EXPECT_DOUBLE_EQ( 0, b() );
    b = 1.23456789;
    EXPECT_DOUBLE_EQ( 1.23456789, b() );
}

template< typename T >
static void test_float32_byte_order( float value, char byte0, char byte1, char byte2, char byte3 )
{
    T a;
    a = value;
    EXPECT_EQ( ( 0xff & byte0 ), ( 0xff & a.data()[0] ) );
    EXPECT_EQ( ( 0xff & byte1 ), ( 0xff & a.data()[1] ) );
    EXPECT_EQ( ( 0xff & byte2 ), ( 0xff & a.data()[2] ) );
    EXPECT_EQ( ( 0xff & byte3 ), ( 0xff & a.data()[3] ) );
}

TEST( test_packed_struct_test, test_float32_byte_order )
{
    test_float32_byte_order< comma::packed::little_endian::float32 >( 5.2, 0x66, 0x66, 0xA6, 0x40 );
    test_float32_byte_order< comma::packed::little_endian::float32 >( -5.2, 0x66, 0x66, 0xA6, 0xC0 );
}

TEST( test_packed_struct_test, test_big_endian_float32_byte_order )
{
    test_float32_byte_order< comma::packed::big_endian::float32 >( 5.2, 0x40, 0xA6, 0x66, 0x66 );
    test_float32_byte_order< comma::packed::big_endian::float32 >( -5.2, 0xC0, 0xA6, 0x66, 0x66 );
}

template< typename T >
static void test_float64_byte_order( double value, char byte0, char byte1, char byte2, char byte3, char byte4, char byte5, char byte6, char byte7 )
{
    T a;
    a = value;
    EXPECT_EQ( ( 0xff & byte0 ), ( 0xff & a.data()[0] ) );
    EXPECT_EQ( ( 0xff & byte1 ), ( 0xff & a.data()[1] ) );
    EXPECT_EQ( ( 0xff & byte2 ), ( 0xff & a.data()[2] ) );
    EXPECT_EQ( ( 0xff & byte3 ), ( 0xff & a.data()[3] ) );
    EXPECT_EQ( ( 0xff & byte4 ), ( 0xff & a.data()[4] ) );
    EXPECT_EQ( ( 0xff & byte5 ), ( 0xff & a.data()[5] ) );
    EXPECT_EQ( ( 0xff & byte6 ), ( 0xff & a.data()[6] ) );
    EXPECT_EQ( ( 0xff & byte7 ), ( 0xff & a.data()[7] ) );
}

TEST( test_packed_struct_test, test_float64_byte_order )
{
    test_float64_byte_order< comma::packed::little_endian::float64 >( 5.2, 0xCD, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0x14, 0x40 );
    test_float64_byte_order< comma::packed::little_endian::float64 >( -5.2, 0xCD, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0x14, 0xC0 );
    test_float64_byte_order< comma::packed::little_endian::float64 >( -1.2e-123, 0x4E, 0x57, 0x04, 0xD1, 0x71, 0x62, 0x69, 0xA6 );
    test_float64_byte_order< comma::packed::little_endian::float64 >( -1.2e+123, 0x21, 0xBD, 0xC3, 0x60, 0x60, 0x0B, 0x7D, 0xD9 );
}

TEST( test_packed_struct_test, test_big_endian_float64_byte_order )
{
    test_float64_byte_order< comma::packed::big_endian::float64 >( 5.2, 0x40, 0x14, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCD );
    test_float64_byte_order< comma::packed::big_endian::float64 >( -5.2, 0xC0, 0x14, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCD );
    test_float64_byte_order< comma::packed::big_endian::float64 >( -1.2e-123, 0xA6, 0x69, 0x62, 0x71, 0xD1, 0x04, 0x57, 0x4E );
    test_float64_byte_order< comma::packed::big_endian::float64 >( -1.2e+123, 0xD9, 0x7D, 0x0B, 0x60, 0x60, 0xC3, 0xBD, 0x21 );
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

TEST( test_packed_ascii_hex, test_pack_value_is_too_large )
{
    char buf1[] = "X";
    char buf2[] = "XX";
    comma::packed::ascii_hex< comma::uint16, 1 > a;
    ASSERT_THROW( a.pack( buf1, 16 ), comma::exception );
    ASSERT_THROW( a.pack( buf2, 16 ), comma::exception );

    comma::packed::ascii_hex< comma::uint16, 2 > b;
    ASSERT_THROW( b.pack( buf2, 256 ), comma::exception );
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

TEST( test_packed_ascii_hex, test_values_from_packed_struct )
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

TEST( test_packed_ascii_hex, test_throw_unexpected_decimal_digit )
{
    ASSERT_THROW( comma::packed::hex_from_int< comma::uint16 >( 16 ), comma::exception);
}

TEST( test_packed_bits, test_reverse_bits_in_byte )
{
    unsigned char x = 0;
    unsigned char expected = 0;
    comma::packed::reverse_bits( x );
    EXPECT_EQ( expected, x );

           x = 0b00000001;
    expected = 0b10000000;
    comma::packed::reverse_bits( x );
    EXPECT_EQ( expected, x );

           x = 0b10000000;
    expected = 0b00000001;
    comma::packed::reverse_bits( x );
    EXPECT_EQ( expected, x );

           x = 0b00010000;
    expected = 0b00001000;
    comma::packed::reverse_bits( x );
    EXPECT_EQ( expected, x );

           x = 0b11111111;
    expected = 0b11111111;
    comma::packed::reverse_bits( x );
    EXPECT_EQ( expected, x );

           x = 0b11111110;
    expected = 0b01111111;
    comma::packed::reverse_bits( x );
    EXPECT_EQ( expected, x );

           x = 0b01111111;
    expected = 0b11111110;
    comma::packed::reverse_bits( x );
    EXPECT_EQ( expected, x );

           x = 0b11101111;
    expected = 0b11110111;
    comma::packed::reverse_bits( x );
    EXPECT_EQ( expected, x );

           x = 0b01010101;
    expected = 0b10101010;
    comma::packed::reverse_bits( x );
    EXPECT_EQ( expected, x );

           x = 0b10101010;
    expected = 0b01010101;
    comma::packed::reverse_bits( x );
    EXPECT_EQ( expected, x );

           x = 0b10011011;
    expected = 0b11011001;
    comma::packed::reverse_bits( x );
    EXPECT_EQ( expected, x );

           x = 0b01000110;
    expected = 0b01100010;
    comma::packed::reverse_bits( x );
    EXPECT_EQ( expected, x );

           x = 0b01000111;
    expected = 0b11100010;
    comma::packed::reverse_bits( x );
    EXPECT_EQ( expected, x );

           x = 0b11010110;
    expected = 0b01101011;
    comma::packed::reverse_bits( x );
    EXPECT_EQ( expected, x );

}

TEST( test_packed_bits, test_reverse_bits_in_uint16 )
{
    comma::uint16 x = 0;
    comma::uint16 expected = 0;
    comma::packed::reverse_bits( x );
    EXPECT_EQ( expected, x );

           x = 0x1;
    expected = 0x8000;
    comma::packed::reverse_bits( x );
    EXPECT_EQ( expected, x );

           x = 0x8000;
    expected = 0x1;
    comma::packed::reverse_bits( x );
    EXPECT_EQ( expected, x );

           x = 0xffff;
    expected = 0xffff;
    comma::packed::reverse_bits( x );
    EXPECT_EQ( expected, x );

           x = 0x1234;
    expected = 0x2c48;
    comma::packed::reverse_bits( x );
    EXPECT_EQ( expected, x );
}

TEST( test_packed_bits, test_reverse_bits_in_uint32 )
{
    comma::uint32 x = 0;
    comma::uint32 expected = 0;
    comma::packed::reverse_bits( x );
    EXPECT_EQ( expected, x );

           x = 0x1;
    expected = 0x80000000;
    comma::packed::reverse_bits( x );
    EXPECT_EQ( expected, x );

           x = 0x80000000;
    expected = 0x1;
    comma::packed::reverse_bits( x );
    EXPECT_EQ( expected, x );

           x = 0xffffffff;
    expected = 0xffffffff;
    comma::packed::reverse_bits( x );
    EXPECT_EQ( expected, x );

           x = 0x1234abcd;
    expected = 0xb3d52c48;
    comma::packed::reverse_bits( x );
    EXPECT_EQ( expected, x );
}

TEST( test_packed_bits, test_reverse_bits_in_uint64 )
{
    comma::uint64 x = 0;
    comma::uint64 expected = 0;
    comma::packed::reverse_bits( x );
    EXPECT_EQ( expected, x );

           x = 0x1;
    expected = 0x8000000000000000;
    comma::packed::reverse_bits( x );
    EXPECT_EQ( expected, x );

           x = 0x8000000000000000;
    expected = 0x1;
    comma::packed::reverse_bits( x );
    EXPECT_EQ( expected, x );

           x = 0xffffffffffffffff;
    expected = 0xffffffffffffffff;
    comma::packed::reverse_bits( x );
    EXPECT_EQ( expected, x );

           x = 0xffffffff00000000;
    expected = 0x00000000ffffffff;
    comma::packed::reverse_bits( x );
    EXPECT_EQ( expected, x );

           x = 0xfffffffffffffffe;
    expected = 0x7fffffffffffffff;
    comma::packed::reverse_bits( x );
    EXPECT_EQ( expected, x );

           x = 0x7fffffffffffffff;
    expected = 0xfffffffffffffffe;
    comma::packed::reverse_bits( x );
    EXPECT_EQ( expected, x );

           x = 0xffffdfffffffffff;
    expected = 0xfffffffffffbffff;
    comma::packed::reverse_bits( x );
    EXPECT_EQ( expected, x );

           x = 0x12345678abcdef01;
    expected = 0x80f7b3d51e6a2c48;
    comma::packed::reverse_bits( x );
    EXPECT_EQ( expected, x );
}


TEST( test_packed_bits, test_packed_double_reversion )
{
    comma::uint64 x = 0x89a3f724e89b7214;
    comma::uint64 expected = x;
    comma::packed::reverse_bits( x );
    comma::packed::reverse_bits( x );
    EXPECT_EQ( expected, x );
}

struct status_bits
{
    status_bits() : a( 0 ), b( 0 ), unused( 0 ), c( 0 ) {}
    unsigned char a: 1, b: 2, unused: 3, c: 2;
};


TEST( test_packed_bits, test_bits )
{
    comma::packed::bits< status_bits > packed_status;
    EXPECT_EQ( 0, packed_status().a );
    EXPECT_EQ( 0, packed_status().b );
    EXPECT_EQ( 0, packed_status().c );
    packed_status = 0b1U;
    EXPECT_EQ( 0b1U, packed_status().a );
    packed_status = 0b10U;
    EXPECT_EQ( 0b1U, packed_status().b );
    packed_status = 0b11000000U;
    EXPECT_EQ( 0b11U, packed_status().c );
}

TEST( test_packed_bits, test_bits_default )
{
    static const unsigned char d = 0b11111111U;
    comma::packed::bits< status_bits, d > packed_status;
    EXPECT_EQ( 0b1U, packed_status().a );
    EXPECT_EQ( 0b11U, packed_status().b );
    EXPECT_EQ( 0b11U, packed_status().c );
    packed_status.fields().a = 0;
    packed_status.fields().b = 0;
    packed_status.fields().c = 0;
    EXPECT_EQ( 0U, packed_status().a );
    EXPECT_EQ( 0U, packed_status().b );
    EXPECT_EQ( 0U, packed_status().c );
    status_bits default_status = packed_status.default_value();
    EXPECT_EQ( 0b1U, default_status.a );
    EXPECT_EQ( 0b11U, default_status.b );
    EXPECT_EQ( 0b11U, default_status.c );
}

TEST( test_packed_bits, test_bits_operators )
{
    {
        static const unsigned char d = 0b11111111U;
        comma::packed::bits< status_bits, d > packed_status;
        packed_status.fields().a = 0;
        packed_status.fields().b = 0;
        packed_status.fields().c = 0;
        comma::packed::bits< status_bits, d > s;
        EXPECT_EQ( 0b1U, s().a );
        EXPECT_EQ( 0b11U, s().b );
        EXPECT_EQ( 0b11U, s().c );
        s = packed_status;
        EXPECT_EQ( 0U, s().a );
        EXPECT_EQ( 0U, s().b );
        EXPECT_EQ( 0U, s().c );
    }
    {
        static const unsigned char d = 0b11111111U;
        status_bits status;
        status.a = 0b0U;
        status.b = 0b01U;
        status.c = 0b10U;
        comma::packed::bits< status_bits, d > s;
        EXPECT_EQ( 0b1U, s().a );
        EXPECT_EQ( 0b11U, s().b );
        EXPECT_EQ( 0b11U, s().c );
        s = status;
        EXPECT_EQ( 0b0U, s().a );
        EXPECT_EQ( 0b01U, s().b );
        EXPECT_EQ( 0b10U, s().c );
    }
}

struct status_bits16
{
    status_bits16() : a( 0 ), b( 0 ), c( 0 ), d( 0 ) {}
    comma::uint16 a: 1, b: 2, : 3, c: 2, d: 8;
};

TEST( test_packed_bits, test_bits16_default )
{
    static const comma::uint16 d = 0b1001100111001100U;
    comma::packed::bits< status_bits16, d > packed_status;
    EXPECT_EQ( 0b0U, packed_status().a );
    EXPECT_EQ( 0b10U, packed_status().b );
    EXPECT_EQ( 0b11U, packed_status().c );
    EXPECT_EQ( 0b10011001U, packed_status().d );
}

TEST( test_packed_bits, test_bits16 )
{
    {
        comma::packed::bits< status_bits16 > packed_status = 0b1001100111001100U;
        EXPECT_EQ( 0b0U, packed_status().a );
        EXPECT_EQ( 0b10U, packed_status().b );
        EXPECT_EQ( 0b11U, packed_status().c );
        EXPECT_EQ( 0b10011001U, packed_status().d );
        EXPECT_EQ( 0b1001100111001100U, packed_status.integer_value() );
    }
    {
        status_bits16 status = comma::packed::bits< status_bits16 >( 0b1001100111001100U )();
        EXPECT_EQ( 0b0U, status.a );
        EXPECT_EQ( 0b10U, status.b );
        EXPECT_EQ( 0b11U, status.c );
        EXPECT_EQ( 0b10011001U, status.d );
    }
}

struct status_bits32
{
    status_bits32() : a( 0 ), b( 0 ), unused1( 0 ), c( 0 ), unused2( 0 ), d( 0 ) {}
    comma::uint32 a: 1, b: 3, unused1: 6, c: 12, unused2: 1, d: 9;
};

TEST( test_packed_bits, reversed_bits_basic_usage )
{
    status_bits32 expected_status;
    expected_status.a = 0b1;
    expected_status.b = 0b100;
    expected_status.c = 0b101111111111;
    expected_status.d = 0b100000011;
    comma::packed::reversed_bits< status_bits32 > packed_status( expected_status );
    std::stringstream ss;
    ss.write( packed_status.data(), sizeof( packed_status ) );
    ss.read( packed_status.data(), sizeof( packed_status ) );
    status_bits32 status = packed_status();
    EXPECT_EQ( expected_status.a, status.a );
    EXPECT_EQ( expected_status.b, status.b );
    EXPECT_EQ( expected_status.c, status.c );
    EXPECT_EQ( expected_status.d, status.d );
}

TEST( test_packed_bits, reversed_bits_contructor )
{
    {
        status_bits32 expected_status;
        expected_status.a = 0b1;
        expected_status.b = 0b100;
        expected_status.c = 0b101111111111;
        expected_status.d = 0b100000011;
        comma::packed::reversed_bits< status_bits32 > packed_status( expected_status );
        status_bits32 status = packed_status();
        EXPECT_EQ( expected_status.a, status.a );
        EXPECT_EQ( expected_status.b, status.b );
        EXPECT_EQ( expected_status.c, status.c );
        EXPECT_EQ( expected_status.d, status.d );
    }
    {
        status_bits32 expected_status;
        expected_status.a = 0b1;
        expected_status.b = 0b100;
        expected_status.c = 0b101111111111;
        expected_status.d = 0b100000011;
        comma::uint32* p = reinterpret_cast< comma::uint32* >( &expected_status );
        comma::uint32 struct_as_integer = *p;
        comma::packed::reversed_bits< status_bits32 > packed_status( struct_as_integer );
        status_bits32 status = packed_status();
        EXPECT_EQ( expected_status.a, status.a );
        EXPECT_EQ( expected_status.b, status.b );
        EXPECT_EQ( expected_status.c, status.c );
        EXPECT_EQ( expected_status.d, status.d );
    }
}

TEST( test_packed_bits, test_reversed_bits_get )
{
    comma::uint32 value = 0b10010000001111111111010110000001UL;
    comma::packed::reversed_bits< status_bits32 > packed_status = *reinterpret_cast< comma::packed::reversed_bits< status_bits32 >* >( &value );
    status_bits32 status = packed_status();
    EXPECT_EQ( 0b1, status.a );
    EXPECT_EQ( 0b100, status.b );
    EXPECT_EQ( 0b101111111111, status.c );
    EXPECT_EQ( 0b100000011, status.d );
}

TEST( test_packed_bits, test_reversed_bits_set_from_struct )
{
    status_bits32 status;
    status.a = 0b1;
    status.b = 0b100;
    status.c = 0b101111111111;
    status.d = 0b100000011;
    comma::packed::reversed_bits< status_bits32 > packed_status;
    packed_status = status;
    comma::uint32 value = *reinterpret_cast< comma::uint32* >( packed_status.data() );
    EXPECT_EQ( 0b10010000001111111111010110000001UL, value );
}

TEST( test_packed_bits, test_reversed_bits_set_from_integer )
{
    status_bits32 status;
    status.a = 0b1;
    status.b = 0b100;
    status.c = 0b101111111111;
    status.d = 0b100000011;
    comma::uint32* p = reinterpret_cast< comma::uint32* >( &status );
    comma::packed::reversed_bits< status_bits32 > packed_status( *p );
    comma::uint32 value = *reinterpret_cast< comma::uint32* >( packed_status.data() );
    EXPECT_EQ( 0b10010000001111111111010110000001UL, value );
    EXPECT_EQ( *p, comma::packed::get_reversed_bits( value ) );
}

TEST( test_packed_bits, test_reversed_bits_default )
{
    {
        comma::packed::reversed_bits< status_bits32 > packed_status;
        status_bits32 status = packed_status();
        EXPECT_EQ( 0, status.a );
        EXPECT_EQ( 0, status.b );
        EXPECT_EQ( 0, status.c );
        EXPECT_EQ( 0, status.d );
    }
    {
        static const comma::uint32 default_struct_as_integer = 0b10000001101011111111110000001001UL;
        status_bits32 default_status;
        default_status.a = 0b1;
        default_status.b = 0b100;
        default_status.c = 0b101111111111;
        default_status.d = 0b100000011;
        comma::uint32* p = reinterpret_cast< comma::uint32* >( &default_status );
        ASSERT_EQ( default_struct_as_integer, *p );

        comma::packed::reversed_bits< status_bits32, default_struct_as_integer > packed_status;
        status_bits32 status = packed_status();
        EXPECT_EQ( default_status.a, status.a );
        EXPECT_EQ( default_status.b, status.b );
        EXPECT_EQ( default_status.c, status.c );
        EXPECT_EQ( default_status.d, status.d );
    }
}

TEST( test_packed_bits, test_reversed_bits_set_from_packed )
{
    status_bits32 status1;
    status1.a = 0b1;
    status1.b = 0b100;
    status1.c = 0b101111111111;
    status1.d = 0b100000011;
    comma::packed::reversed_bits< status_bits32 > packed_status1;
    comma::packed::reversed_bits< status_bits32 > packed_status2;
    packed_status1 = status1;
    packed_status2 = packed_status1;
    status_bits32 status2 = packed_status2();
    EXPECT_EQ( status1.a, status2.a );
    EXPECT_EQ( status1.b, status2.b );
    EXPECT_EQ( status1.c, status2.c );
    EXPECT_EQ( status1.d, status2.d );
}

int main( int argc, char *argv[] )
{
    ::testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS();
}
