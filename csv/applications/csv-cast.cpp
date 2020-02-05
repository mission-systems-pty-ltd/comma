// This file is part of comma, a generic and flexible library
// Copyright (c) 2014 The University of Sydney
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


/// @author Vinny Do

#ifdef WIN32
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#endif

#include <cstring>
#include <iostream>
#include "../../application/command_line_options.h"
#include "../../application/contact_info.h"
#include "../../base/exception.h"
#include "../../csv/format.h"

static const std::string app_name = "csv-cast";

static void usage()
{
    std::cerr << "reads binary in the given input format and writes binary in the given output format" << std::endl;
    std::cerr << std::endl;
    std::cerr << "usage: cat blah.bin | " << app_name << " <input-format> <output-format> [option...] > blah.1.bin" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options:" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    --binary,-b,--from: input binary format" << std::endl;
    std::cerr << "    --output-binary,--output,-o,--to: output binary format" << std::endl;
    std::cerr << "    --flush: flush stdout after each record" << std::endl;
    std::cerr << "    --force: allow narrowing conversions" << std::endl;
    std::cerr << std::endl;
    std::cerr << comma::csv::format::usage() << std::endl;
    std::cerr << "notes:" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    type conversion with time types is not supported (s[length] to [l]t is supported)" << std::endl;
    std::cerr << "    lexical casting from s[length] is supported (conversion to s[length] is not)" << std::endl;
    std::cerr << "    narrowing conversion is considered an error. use --force to allow." << std::endl;
    std::cerr << std::endl;
    std::cerr << "examples:" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    convert floats to doubles" << std::endl;
    std::cerr << "        cat blah.bin | csv-cast t,3ui,2f,d t,3ui,3d > new.bin" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    convert t,d,f to t,f,d (creates sample binary data)" << std::endl;
    std::cerr << "        echo {0..9}.2345789,3.1415 | fmt -1 | csv-time-stamp | csv-to-bin t,d,f | csv-cast t,d,f t,f,d --force | csv-from-bin t,f,d" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    lexical cast, convert s[22],s[10],s[6] to t,2d (creates sample binary data)" << std::endl;
    std::cerr << "        echo {0..9}.2345789,3.1415 | fmt -1 | csv-time-stamp | csv-to-bin s[22],s[10],s[6] | csv-cast s[22],s[10],s[6] t,2d | csv-from-bin t,2d" << std::endl;
    std::cerr << std::endl;
    std::cerr << comma::contact_info << std::endl;
    std::cerr << std::endl;
}

static void check_conversions( const comma::csv::format& iformat, const comma::csv::format& oformat, const bool force )
{
    if( iformat.count() != oformat.count() ) { COMMA_THROW( comma::exception, "format count mismatch; input format: " << iformat.count() << ", output format: " << oformat.count() ); }
    unsigned int ioffset = 0;
    unsigned int icount = 0;
    unsigned int ooffset = 0;
    unsigned int ocount = 0;
    for( unsigned int i = 0; i < iformat.count(); ++i, ++icount, ++ocount )
    {
        if( icount >= iformat.elements()[ ioffset ].count ) { icount = 0; ++ioffset; }
        if( ocount >= oformat.elements()[ ooffset ].count ) { ocount = 0; ++ooffset; }
        comma::csv::format::types_enum from_type = iformat.elements()[ ioffset ].type;
        comma::csv::format::types_enum to_type = oformat.elements()[ ooffset ].type;

        // check type conversions
        if( from_type == to_type || from_type == comma::csv::format::fixed_string ) { continue; }
        bool valid = true;
        switch( to_type )
        {
            case comma::csv::format::fixed_string:
            case comma::csv::format::time:
            case comma::csv::format::long_time:         valid = false; break;
            default:                                    break;
        }
        if( from_type == comma::csv::format::time ) { valid = false; }
        if( !valid ) { COMMA_THROW( comma::exception, "type conversion from " << comma::csv::format::to_format( from_type ) << " to " << comma::csv::format::to_format( to_type ) << " is not supported" ); }
        
        // check narrowing conversions
        if( force ) { continue; }
        switch( from_type )
        {
            case comma::csv::format::double_t:
                if( to_type == comma::csv::format::float_t ) { valid = false; break; }
            case comma::csv::format::float_t:
                switch( to_type )
                {
                    case comma::csv::format::char_t:
                    case comma::csv::format::int8:
                    case comma::csv::format::uint8:
                    case comma::csv::format::int16:
                    case comma::csv::format::uint16:
                    case comma::csv::format::int32:
                    case comma::csv::format::uint32:
                    case comma::csv::format::int64:
                    case comma::csv::format::uint64:
                        valid = false;
                        break;
                    default:
                        break;
                }
                break;
            case comma::csv::format::int16:
            case comma::csv::format::uint16:
            case comma::csv::format::int32:
            case comma::csv::format::uint32:
            case comma::csv::format::int64:
            case comma::csv::format::uint64:
                if( oformat.elements()[ ioffset ].size < iformat.elements()[ ioffset ].size ) { valid = false; }
                break;
            default:
                break;
        }
        if( !valid ) { COMMA_THROW( comma::exception, "error: narrowing conversion from " << comma::csv::format::to_format( from_type ) << " to " << comma::csv::format::to_format( to_type ) << ". use --force to allow." ); }
    }
}

template< typename To >
static void lexical_cast( const std::string& in, char* out ) { comma::csv::format::traits< To >::to_bin( boost::lexical_cast< To >( comma::csv::format::traits< std::string >::from_bin( &in[0], in.size() ) ), out ); }

template<>
void lexical_cast< boost::posix_time::ptime >( const std::string& in, char* out )
{
    boost::posix_time::ptime t;
    try { t = boost::posix_time::from_iso_string( in ); }
    catch ( ... ) { t = boost::posix_time::not_a_date_time; }
    comma::csv::format::traits< boost::posix_time::ptime >::to_bin( t, out );
}

static void lexical_cast_long_time( const std::string& in, char* out )
{
    boost::posix_time::ptime t;
    try { t = boost::posix_time::from_iso_string( in ); }
    catch ( ... ) { t = boost::posix_time::not_a_date_time; }
    comma::csv::format::traits< boost::posix_time::ptime, comma::csv::format::long_time >::to_bin( t, out );
}

static void lexical_cast( const comma::csv::format::types_enum to_type, const std::string& in, char* out )
{
    switch( to_type )
    {
        case comma::csv::format::int8:          lexical_cast< char >( in, out ); break;
        case comma::csv::format::uint8:         lexical_cast< unsigned char >( in, out ); break;
        case comma::csv::format::int16:         lexical_cast< comma::int16 >( in, out ); break;
        case comma::csv::format::uint16:        lexical_cast< comma::uint16 >( in, out ); break;
        case comma::csv::format::int32:         lexical_cast< comma::int32 >( in, out ); break;
        case comma::csv::format::uint32:        lexical_cast< comma::uint32 >( in, out ); break;
        case comma::csv::format::int64:         lexical_cast< comma::int64 >( in, out ); break;
        case comma::csv::format::uint64:        lexical_cast< comma::uint64 >( in, out ); break;
        case comma::csv::format::char_t:        lexical_cast< char >( in, out ); break;
        case comma::csv::format::float_t:       lexical_cast< float >( in, out ); break;
        case comma::csv::format::double_t:      lexical_cast< double >( in, out ); break;
        case comma::csv::format::time:          lexical_cast< boost::posix_time::ptime >( in, out ); break;
        case comma::csv::format::long_time:     lexical_cast_long_time( in, out ); break;
        default:                                COMMA_THROW( comma::exception, "type conversion from fixed_string to " << comma::csv::format::to_format( to_type ) << " is not supported" ); break;
    }
}

template< typename From, typename To >
static void cast( const char* in, char* out ) { comma::csv::format::traits< To >::to_bin( comma::csv::format::traits< From >::from_bin( in ), out ); }

template < typename From >
static void cast( const comma::csv::format::types_enum to_type, const char* in, char* out  )
{
    switch( to_type )
    {
        case comma::csv::format::int8:          cast< From, char >( in, out ); break;
        case comma::csv::format::uint8:         cast< From, unsigned char >( in, out ); break;
        case comma::csv::format::int16:         cast< From, comma::int16 >( in, out ); break;
        case comma::csv::format::uint16:        cast< From, comma::uint16 >( in, out ); break;
        case comma::csv::format::int32:         cast< From, comma::int32 >( in, out ); break;
        case comma::csv::format::uint32:        cast< From, comma::uint32 >( in, out ); break;
        case comma::csv::format::int64:         cast< From, comma::int64 >( in, out ); break;
        case comma::csv::format::uint64:        cast< From, comma::uint64 >( in, out ); break;
        case comma::csv::format::char_t:        cast< From, char >( in, out ); break;
        case comma::csv::format::float_t:       cast< From, float >( in, out ); break;
        case comma::csv::format::double_t:      cast< From, double >( in, out ); break;
        default:                                COMMA_THROW( comma::exception, "type conversion to " << comma::csv::format::to_format( to_type ) << " is not supported" ); break;
    }
}

void cast( const comma::csv::format& iformat, const std::vector< char >& input, const comma::csv::format& oformat, std::vector< char >& output )
{
    const char* in = &input[0];
    unsigned int ioffset = 0;
    unsigned int icount = 0;
    char* out = &output[0];
    unsigned int ooffset = 0;
    unsigned int ocount = 0;
    for( unsigned int i = 0; i < iformat.count(); ++i, ++icount, ++ocount )
    {
        if( icount >= iformat.elements()[ ioffset ].count ) { icount = 0; ++ioffset; }
        if( ocount >= oformat.elements()[ ooffset ].count ) { ocount = 0; ++ooffset; }
        comma::csv::format::types_enum from_type = iformat.elements()[ ioffset ].type;
        comma::csv::format::types_enum to_type = oformat.elements()[ ooffset ].type;
        if( from_type == to_type ) { std::memcpy( out, in, iformat.elements()[ ioffset ].size ); }
        else
        {
            switch( from_type )
            {
                case comma::csv::format::int8:          cast< char >( to_type, in, out ); break;
                case comma::csv::format::uint8:         cast< unsigned char >( to_type, in, out ); break;
                case comma::csv::format::int16:         cast< comma::int16 >( to_type, in, out ); break;
                case comma::csv::format::uint16:        cast< comma::uint16 >( to_type, in, out ); break;
                case comma::csv::format::int32:         cast< comma::int32 >( to_type, in, out ); break;
                case comma::csv::format::uint32:        cast< comma::uint32 >( to_type, in, out ); break;
                case comma::csv::format::int64:         cast< comma::int64 >( to_type, in, out ); break;
                case comma::csv::format::uint64:        cast< comma::uint64 >( to_type, in, out ); break;
                case comma::csv::format::char_t:        cast< char >( to_type, in, out ); break;
                case comma::csv::format::float_t:       cast< float >( to_type, in, out ); break;
                case comma::csv::format::double_t:      cast< double >( to_type, in, out ); break;
                case comma::csv::format::fixed_string:  lexical_cast( to_type, std::string( in, iformat.elements()[ ioffset ].size ), out ); break;
                default:                                COMMA_THROW( comma::exception, "type conversion from " << comma::csv::format::to_format( from_type ) << " to " << comma::csv::format::to_format( to_type ) << " is not supported" ); break;
            }
        }
        in += iformat.elements()[ ioffset ].size;
        out += oformat.elements()[ ooffset ].size;
    }
}

int main( int ac, char** av )
{
#ifdef WIN32
    _setmode( _fileno( stdin ), _O_BINARY );
    _setmode( _fileno( stdout ), _O_BINARY );
#endif
    try
    {
        comma::command_line_options options( ac, av );
        if( options.exists( "--help,-h" ) ) { usage(); return 0; }
        if( ac < 3 ) { usage(); return 1; }
        comma::csv::format iformat( options.value< std::string >( "--binary,-b,--from", av[1] ) );
        comma::csv::format oformat( options.value< std::string >( "--output-binary,--output,-o,--to", av[2] ) );
        check_conversions( iformat, oformat, options.exists( "--force" ) );
        bool flush = options.exists( "--flush" );
        std::vector< char > in( iformat.size() );
        std::vector< char > out( oformat.size() );
        if( !flush ) { std::cin.tie( NULL ); }
        while( std::cin.good() )
        {
            std::cin.read( &in[0], iformat.size() );
            if( std::cin.gcount() == 0 ) { break; }
            if( std::cin.gcount() < static_cast< int >( iformat.size() ) ) { COMMA_THROW( comma::exception, "expected " << iformat.size() << " bytes, got only " << std::cin.gcount() ); }
            cast( iformat, in, oformat, out );
            std::cout.write( &out[0], oformat.size() );
            if( flush ) { std::cout.flush(); }
        }
        return 0;
    }
    catch( std::exception& ex ) { std::cerr << app_name << ": " << ex.what() << std::endl; }
    catch( ... ) { std::cerr << app_name << ": unknown exception" << std::endl; }
    return 1;
}
