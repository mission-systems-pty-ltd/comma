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


/// @author vsevolod vlaskine

#include <string.h>
#include <iostream>
#include <string>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <comma/application/contact_info.h>
#include <comma/application/command_line_options.h>
#include <comma/base/exception.h>
#include <comma/base/types.h>
#include <comma/string/string.h>
#include <comma/csv/impl/epoch.h>

static void usage()
{
    std::cerr << std::endl;
    std::cerr << "Convert to and from seconds since epoch as double to ISO string" << std::endl;
    std::cerr << "Usage: cat log.csv | csv-time <options> > converted.csv" << std::endl;
    std::cerr << "<options>:" << std::endl;
    std::cerr << "    --to-seconds,--sec,-s" << std::endl;
    std::cerr << "    --to-iso-string,--iso,-i" << std::endl;
    std::cerr << "    --delimiter,-d <delimiter> : default: ','" << std::endl;
    std::cerr << "    --fields <fields> : time field numbers as in \"cut\"" << std::endl;
    std::cerr << "                        e.g. \"1,5,7\"; default: 1" << std::endl;
    std::cerr << std::endl;
    std::cerr << comma::contact_info << std::endl;
    std::cerr << std::endl;
    exit( -1 );
}

int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av );
        if( options.exists( "--help" ) || options.exists( "-h" ) || ac == 1 ) { usage(); }
        bool from = options.exists( "--to-iso-string" ) || options.exists( "--iso" ) || options.exists( "-i" );
        bool to = options.exists( "--to-seconds" ) || options.exists( "--sec" ) || options.exists( "-s" );
        if( to && from ) { COMMA_THROW( comma::exception, "cannot have both --to-seconds and --to-iso-string" ); }
        if( !to && !from ) { COMMA_THROW( comma::exception, "please specify either --to-seconds or --to-iso-string" ); }
        char delimiter = options.exists( "-d" ) ? options.value( "-d", ',' ) : options.value( "--delimiter", ',' );
        std::vector< std::string > fields = comma::split( options.value< std::string >( "--fields", "1" ), ',' );
        std::vector< std::size_t > indices( fields.size() );
        unsigned int minSize = 0;
        for( unsigned int i = 0; i < fields.size(); ++i )
        {
            unsigned int v = boost::lexical_cast< unsigned int >( fields[i] );
            if( v > minSize ) { minSize = v; }
            indices[i] = v - 1;
        }
        while( std::cin.good() && !std::cin.eof() )
        {
            std::string s;
            std::getline( std::cin, s );
            if( !s.empty() && *s.rbegin() == '\r' ) { s = s.substr( 0, s.length() - 1 ); } // windows... sigh...
            if( s.length() == 0 ) { continue; }
            std::vector< std::string > v = comma::split( s, delimiter );
            if( v.size() < minSize ) { COMMA_THROW( comma::exception, "expected at least " << minSize << " '" << delimiter << "'-separated values; got [" << s << "]" ); }
            if( from )
            {
                for( unsigned int i = 0; i < indices.size(); ++i )
                {
                    std::vector< std::string > w = comma::split( v[ indices[i] ], '.' );
                    boost::posix_time::ptime t;
                    switch( w.size() )
                    {
                        case 1:
                        {
                            t = boost::posix_time::ptime( comma::csv::impl::epoch, boost::posix_time::seconds(boost::lexical_cast< long >( w[0] ) ) );
                            break;
                        }
                        case 2:
                        {
                            if( w[1].length() > 9 ) { COMMA_THROW( comma::exception, "invalid nanoseconds in [" << v[ indices[i] ] << "]" ); }
                            std::string n = "000000000";
                            ::memcpy( &n[0], &w[1][0], w[1].length() ); 
                            comma::uint32 nanoseconds = boost::lexical_cast< comma::uint32 >( n );
                            t = boost::posix_time::ptime(comma::csv::impl::epoch, boost::posix_time::seconds( boost::lexical_cast< long >( w[0] ) ) + boost::posix_time::microseconds(nanoseconds/1000));
                            break;
                        }
                        default:
                            COMMA_THROW( comma::exception, "expected seconds as double; got [" << v[ indices[i] ] << "]" );
                    }
                    v[ indices[i] ] = boost::posix_time::to_iso_string( t );
                }
            }
            else
            {
                for( unsigned int i = 0; i < indices.size(); ++i )
                {
                    boost::posix_time::ptime t = boost::posix_time::from_iso_string( v[ indices[i] ] );
                    const boost::posix_time::ptime base( comma::csv::impl::epoch );
                    const boost::posix_time::time_duration posix = t - base;
                    comma::uint64 seconds = posix.total_seconds();
                    comma::uint32 nanoseconds = static_cast< unsigned int >( posix.total_microseconds() % 1000000 ) * 1000;
                    std::ostringstream oss;
                    oss << seconds;
                    if( nanoseconds > 0 )
                    {
                        oss << '.';
                        oss.width( 9 );
                        oss.fill( '0' );
                        oss << nanoseconds;
                    }
                    v[ indices[i] ] = oss.str();
                }                
            }
            std::cout << v[0];
            for( unsigned int i = 1; i < v.size(); ++i ) { std::cout << delimiter << v[i]; }
            std::cout << std::endl;
        }
        return 0;     
    }
    catch( std::exception& ex ) { std::cerr << "csv-time: " << ex.what() << std::endl; }
    catch( ... ) { std::cerr << "csv-time: unknown exception" << std::endl; }
    usage();
}
