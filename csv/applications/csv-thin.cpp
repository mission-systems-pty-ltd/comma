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

/// @author vsevolod vlaskine

#ifdef WIN32
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#endif

#include <iostream>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>
#include "../../application/command_line_options.h"
#include "../../application/contact_info.h"
#include "../../base/exception.h"
#include "../../base/types.h"
#include "../../io/file_descriptor.h"
#include "../../math/compare.h"
#include "../../csv/options.h"
#include "../../csv/stream.h"
#include "../../visiting/traits.h"

using namespace comma;

static void usage(bool detail=false)
{
    std::cerr << std::endl;
    std::cerr << "Read input data and thin them down by the given percentage;" << std::endl;
    std::cerr << "buffer handling optimized for a high-output producer" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Usage: cat full.csv | csv-thin <rate> [<options>] > thinned.csv" << std::endl;
    std::cerr << std::endl;
    std::cerr << "e.g. output 70% of data:  cat full.csv | csv-thin 0.7 > thinned.csv" << std::endl;
    std::cerr << std::endl;
    std::cerr << "<options>" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    --binary,-b <size>: if given, data is packets of fixed size" << std::endl;
    std::cerr << "                        alternatively use --size" << std::endl;
    std::cerr << "    --deterministic,-d: if given, input is downsampled by a factor of int(1 / <rate>)." << std::endl;
    std::cerr << "                        That is, if <rate> is 0.33, output every third packet." << std::endl;
    std::cerr << "                        Default is to output each packet with a probability of <rate>." << std::endl;
    std::cerr << "   --fps,--frames-per-second <d>: when specified use time rate for thinning, output at time rate of <d> records per second; not to be used with <rate>" << std::endl;
    std::cerr << "                        if --fields is specified and it contains 't' then it will be used as timestamp for thinning otherwise it uses real time of reading input" << std::endl;
    std::cerr << "                            other standard csv options may apply if t field is used; e.g. --binary=<format> if input is binary" << std::endl;
    std::cerr << "    --size,-s <size>: if given, data is packets of fixed size" << std::endl;
    std::cerr << "                      otherwise data is expected line-wise" << std::endl;
    std::cerr << "                      alternatively use --binary" << std::endl;
    std::cerr << std::endl;
    if( detail )
    {
        std::cerr << "csv options:" << std::endl;
        std::cerr<< comma::csv::options::usage() << std::endl;
        std::cerr << std::endl;
    }
    else
    {
        std::cerr << "use -v or --verbose to see more detail" << std::endl;
        std::cerr << std::endl;
    }
    std::cerr << comma::contact_info << std::endl;
    std::cerr << std::endl;
    exit( 1 );
}

static double rate;
static bool deterministic;
boost::optional<double> fps;

static bool ignore()
{
    if( deterministic )
    {
        /*
        static unsigned int count = 1;
        static unsigned int kept = 1;
        if( double( kept ) / count > rate ) { ++count; return false; }
        if( kept == rate * count ) { count = 0; kept = 0; }
        ++kept;
        ++count;
        return true;
        */
        
        static unsigned long long size = 1000000;
        static unsigned long long step = 0;
        static unsigned long long count = 0;
        ++count;
        if( count < ( step + 1 ) / rate ) { return true; }
        ++step;
        if( step == size )
        {
            count = 0;
            step = 0;
        }
        return false;
    }
    if( fps )
    {
        static boost::posix_time::time_duration period=boost::posix_time::microseconds( 1e6 / *fps );
        static boost::posix_time::ptime last_time;
        boost::posix_time::ptime now = boost::posix_time::microsec_clock::universal_time();
        if( last_time.is_not_a_date_time() || ( now >= last_time + period ) )
        {
            last_time = now;
            return false;
        }
        return true;
    }
    static boost::mt19937 rng;
    static boost::uniform_real<> dist( 0, 1 );
    static boost::variate_generator< boost::mt19937&, boost::uniform_real<> > random( rng, dist );
    static bool do_ignore = comma::math::less( rate, 1.0 );
    return do_ignore && random() > rate;
}

struct input_t { boost::posix_time::ptime t; };

namespace comma { namespace visiting {

template <> struct traits< input_t >
{
    template< typename K, typename V > static void visit( const K& k, const input_t& p, V& v ) { v.apply( "t", p.t ); }
    template< typename K, typename V > static void visit( const K& k, input_t& p, V& v ) { v.apply( "t", p.t ); }
};

} } //namespace comma { namespace visiting {


int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av, usage );
        bool binary = options.exists( "--size,-s,--binary,-b" );
        deterministic = options.exists( "--deterministic,-d" );
        fps=options.optional<double>("--fps,--frames-per-second");
        #ifdef WIN32
        if( binary ) { _setmode( _fileno( stdin ), _O_BINARY ); _setmode( _fileno( stdout ), _O_BINARY ); }
        #endif
        
        if( fps )
        {
            std::string fields=options.value<std::string>("--fields","");
            if(!fields.empty())
            {
                std::vector<std::string> ff=split(fields, ",");
                if(std::find(ff.begin(), ff.end(), "t") != ff.end())
                {
                    comma::csv::options csv(options);
                    comma::csv::input_stream<input_t> is(std::cin, csv);
                    static boost::posix_time::time_duration period=boost::posix_time::microseconds( 1e6 / *fps );
                    static boost::posix_time::ptime last_time;
                    while(std::cin.good())
                    {
                        //read a record
                        const input_t* input=is.read();
                        if(!input) { break; }
                        boost::posix_time::ptime now = input->t;
                        if(last_time.is_not_a_date_time() || (now >= last_time + period) )
                        {
                            last_time=now;
                            //write output
                            if(is.is_binary())
                                std::cout.write(is.binary().last(), is.binary().size());
                            else
                                std::cout << comma::join( is.ascii().last(), is.ascii().ascii().delimiter() )<< std::endl;
                        }
                    }
                    return 0;
                }
            }

        }
        else //!fps
        {
            std::vector< std::string > v = options.unnamed( "--deterministic,-d", "-.*" );
            if( v.empty() ) { std::cerr << "csv-thin: please specify rate" << std::endl; usage(); }
            rate = boost::lexical_cast< double >( v[0] );
            if( comma::math::less( rate, 0 ) || comma::math::less( 1, rate ) ) { std::cerr << "csv-thin: expected rate between 0 and 1, got " << rate << std::endl; usage(); }
        }

        if( binary ) // quick and dirty, improve performance by reading larger buffer
        {
            std::size_t size = options.value( "--size,-s", 0u );
            std::string format_string = options.value< std::string >( "--binary,-b", "" );
            boost::optional< comma::csv::format > f;
            if( !format_string.empty() ) { f.reset( comma::csv::format( format_string ) ); }
            if( !size ) { size = f->size(); }
            if( f && f->size() != size ) { std::cerr << "csv-thin: expected consistent size, got --size " << size << " and --binary of size " << f->size() << std::endl; return 1; }
            //fps needs real time of reading input record, so we can't buffer it
            unsigned int factor = fps ? 1 : 65536 / size; // arbitrary
            if( factor == 0 ) { factor = 1; }
            std::vector< char > buf( size * factor );
            #ifdef WIN32
            while( std::cin.good() && !std::cin.eof() )
            {
                // it all does not seem to work: in_avail() always returns 0
                //std::streamsize available = std::cin.rdbuf()->in_avail();
                //if( available < 0 ) { continue; }
                //if( available > 0 ) { std::cerr << "available = " << available << std::endl; }
                //std::size_t e = available < int( size ) ? size : available - available % size;
                std::cin.read( &buf[0], size ); // quick and dirty
                if( std::cin.gcount() <= 0 ) { break; }
                if( std::cin.gcount() < int( size ) ) { std::cerr << "csv-thin: expected " << size << " bytes; got only " << std::cin.gcount() << std::endl; exit( 1 ); }
                { if( !ignore() ) { std::cout.write( &buf[0], size ); std::cout.flush(); } }
            }
            #else
            char* cur = &buf[0];
            unsigned int offset = 0;
            unsigned int capacity = buf.size();
            while( std::cin.good() && !std::cin.eof() )
            {
                int count = ::read( comma::io::stdin_fd, cur + offset, capacity );
                if( count <= 0 )
                {
                    if( offset != 0 ) { std::cerr << "csv-thin: expected at least " << size << " bytes, got only " << offset << std::endl; return 1; }
                    break;
                }
                offset += count;
                capacity -= count;
                for( ; offset >= size; cur += size, offset -= size )
                {
                    if( !ignore() ) { std::cout.write( cur, size ); }
                }
                if( capacity == 0 ) { cur = &buf[0]; offset = 0; capacity = buf.size(); }
                std::cout.flush();
            }
            #endif
        }
        else
        {
            std::string line;
            while( std::cin.good() && !std::cin.eof() )
            {
                std::getline( std::cin, line );
                if( !line.empty() && !ignore() ) { std::cout << line << std::endl; }
            }
        }
        return 0;
    }
    catch( std::exception& ex ) { std::cerr << "csv-size: " << ex.what() << std::endl; }
    catch( ... ) { std::cerr << "csv-size: unknown exception" << std::endl; }
    usage();
}

// // This file is part of comma, a generic and flexible library
// // Copyright (c) 2011 The University of Sydney
// // All rights reserved.
// //
// // Redistribution and use in source and binary forms, with or without
// // modification, are permitted provided that the following conditions are met:
// // 1. Redistributions of source code must retain the above copyright
// //    notice, this list of conditions and the following disclaimer.
// // 2. Redistributions in binary form must reproduce the above copyright
// //    notice, this list of conditions and the following disclaimer in the
// //    documentation and/or other materials provided with the distribution.
// // 3. All advertising materials mentioning features or use of this software
// //    must display the following acknowledgement:
// //    This product includes software developed by the University of Sydney.
// // 4. Neither the name of the University of Sydney nor the
// //    names of its contributors may be used to endorse or promote products
// //    derived from this software without specific prior written permission.
// //
// // NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
// // GRANTED BY THIS LICENSE.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
// // HOLDERS AND CONTRIBUTORS \"AS IS\" AND ANY EXPRESS OR IMPLIED
// // WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// // MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// // DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// // LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// // CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// // SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
// // BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// // WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// // OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
// // IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// /// @author vsevolod vlaskine
//
// #ifdef WIN32
// #include <stdio.h>
// #include <fcntl.h>
// #include <io.h>
// #endif
//
// #include <iostream>
// #include <boost/random/mersenne_twister.hpp>
// #include <boost/random/uniform_real.hpp>
// #include <boost/random/variate_generator.hpp>
// #include <comma/application/command_line_options.h>
// #include <comma/application/contact_info.h>
// #include <comma/base/exception.h>
// #include <comma/base/types.h>
// #include <comma/io/file_descriptor.h>
// #include <comma/math/compare.h>
//
// using namespace comma;
//
// static void usage()
// {
//     std::cerr << std::endl;
//     std::cerr << "Read input data and thin them down by the given percentage;" << std::endl;
//     std::cerr << "buffer handling optimized for a high-output producer" << std::endl;
//     std::cerr << std::endl;
//     std::cerr << "Usage: cat full.csv | csv-thin <rate> [<options>] > thinned.csv" << std::endl;
//     std::cerr << std::endl;
//     std::cerr << "e.g. output 70% of data:  cat full.csv | csv-thin 0.7 > thinned.csv" << std::endl;
//     std::cerr << std::endl;
//     std::cerr << "<options>" << std::endl;
//     std::cerr << std::endl;
//     std::cerr << "    --size,-s <size>: if given, data is packets of fixed size" << std::endl;
//     std::cerr << "                      otherwise data is line-based" << std::endl;
//     std::cerr << "    --deterministic,-d: if given, input is downsampled by a factor of int(1 / <rate>)." << std::endl;
//     std::cerr << "                     That is, if <rate> is 0.33, output every third packet." << std::endl;
//     std::cerr << "                     Default is to output each packet with a probability of <rate>." << std::endl;
//     std::cerr << std::endl;
//     std::cerr << comma::contact_info << std::endl;
//     std::cerr << std::endl;
//     exit( 1 );
// }
//
// static double rate;
// static bool deterministic;
// static unsigned long long count_size;
//
// static bool ignore()
// {
//     if( deterministic )
//     {
//         static unsigned long long count = count_size - 1;
//         if( ++count == count_size ) { count = 0; }
//         if(rate<0.5)
//         {
//             return count != 0;
//         }
//         else
//         {
//             return count == 0;
//         }
//     }
//     static boost::mt19937 rng;
//     static boost::uniform_real<> dist( 0, 1 );
//     static boost::variate_generator< boost::mt19937&, boost::uniform_real<> > random( rng, dist );
//     static bool do_ignore = comma::math::less( rate, 1.0 );
//     return do_ignore && random() > rate;
//
// }
//
// int main( int ac, char** av )
// {
//     try
//     {
//         comma::command_line_options options( ac, av );
//         if( options.exists( "--help,-h" ) || ac == 1 ) { usage(); }
//         bool binary = options.exists( "--size,-s" );
//         deterministic = options.exists( "--deterministic,-d" );
//         std::size_t size = options.value( "--size,-s", 0u );
//         #ifdef WIN32
//         if( binary ) { _setmode( _fileno( stdin ), _O_BINARY ); _setmode( _fileno( stdout ), _O_BINARY ); }
//         #endif
//         std::vector< std::string > v = options.unnamed( "--deterministic,-d", "-.*" );
//         if( v.empty() ) { std::cerr << "csv-thin: please specify rate" << std::endl; usage(); }
//         rate = boost::lexical_cast< double >( v[0] );
//         if( comma::math::less( rate, 0 ) || comma::math::less( 1, rate ) ) { std::cerr << "csv-thin: expected rate between 0 and 1, got " << rate << std::endl; usage(); }
//
//         if( deterministic )
//         {
//             if(rate<0.5)
//             {
//                 count_size = static_cast< unsigned long long >( 1.0 / rate );
//             }
//             else
//             {
//                 count_size = static_cast< unsigned long long >( 1.0 / (1-rate) );
//             }
//         }
//
//         if( binary ) // quick and dirty, improve performance by reading larger buffer
//         {
//             unsigned int factor = 65536 / size; // arbitrary
//             if( factor == 0 ) { factor = 1; }
//             std::vector< char > buf( size * factor );
//             #ifdef WIN32
//             while( std::cin.good() && !std::cin.eof() )
//             {
//                 // it all does not seem to work: in_avail() always returns 0
//                 //std::streamsize available = std::cin.rdbuf()->in_avail();
//                 //if( available < 0 ) { continue; }
//                 //if( available > 0 ) { std::cerr << "available = " << available << std::endl; }
//                 //std::size_t e = available < int( size ) ? size : available - available % size;
//                 std::cin.read( &buf[0], size ); // quick and dirty
//                 if( std::cin.gcount() <= 0 ) { break; }
//                 if( std::cin.gcount() < int( size ) ) { std::cerr << "csv-thin: expected " << size << " bytes; got only " << std::cin.gcount() << std::endl; exit( 1 ); }
//                 { if( !ignore() ) { std::cout.write( &buf[0], size ); std::cout.flush(); } }
//             }
//             #else
//             char* cur = &buf[0];
//             unsigned int offset = 0;
//             unsigned int capacity = buf.size();
//             while( std::cin.good() && !std::cin.eof() )
//             {
//                 int count = ::read( comma::io::stdin_fd, cur + offset, capacity );
//                 if( count <= 0 )
//                 {
//                     if( offset != 0 ) { std::cerr << "csv-thin: expected at least " << size << " bytes, got only " << offset << std::endl; return 1; }
//                     break;
//                 }
//                 offset += count;
//                 capacity -= count;
//                 for( ; offset >= size; cur += size, offset -= size )
//                 {
//                     if( !ignore() ) { std::cout.write( cur, size ); }
//                 }
//                 if( capacity == 0 ) { cur = &buf[0]; offset = 0; capacity = buf.size(); }
//                 std::cout.flush();
//             }
//             #endif
//         }
//         else
//         {
//             std::string line;
//             while( std::cin.good() && !std::cin.eof() )
//             {
//                 std::getline( std::cin, line );
//                 if( !line.empty() && !ignore() ) { std::cout << line << std::endl; }
//             }
//         }
//         return 0;
//     }
//     catch( std::exception& ex ) { std::cerr << "csv-size: " << ex.what() << std::endl; }
//     catch( ... ) { std::cerr << "csv-size: unknown exception" << std::endl; }
//     usage();
// }
