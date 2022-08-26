// Copyright (c) 2011 The University of Sydney

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
#include "../../base/exception.h"
#include "../../base/types.h"
#include "../../io/file_descriptor.h"
#include "../../math/compare.h"
#include "../../csv/options.h"
#include "../../csv/stream.h"
#include "../../visiting/traits.h"

using namespace comma;

static void usage( bool verbose = false )
{
    std::cerr << std::endl;
    std::cerr << "read input data and thin them down by the given percentage;" << std::endl;
    std::cerr << "buffer handling optimized for a high-output producer" << std::endl;
    std::cerr << std::endl;
    std::cerr << "usage: cat full.csv | csv-thin [<rate>] [<options>] > thinned.csv" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options" << std::endl;
    std::cerr << "    --binary,-b=<format>: data is packets of fixed size given by <format>" << std::endl;
    std::cerr << "                          alternatively use --size" << std::endl;
    std::cerr << "    --deterministic,-d: input is downsampled by a factor of int( 1 / <rate> )" << std::endl;
    std::cerr << "                        that is, if <rate> is 0.33, output every third packet" << std::endl;
    std::cerr << "                        default is to output each packet with a probability of <rate>" << std::endl;
    std::cerr << "    --fields=<fields>: use timestamp in fields to determine time for --period" << std::endl;
    std::cerr << "    --invert,-i; invert selection logic; e.g. to split data" << std::endl;
    std::cerr << "    --period=<n>: output once every <n> seconds, ignores <rate>" << std::endl;
    std::cerr << "    --size,-s=<size>: data is packets of fixed size, otherwise data is expected" << std::endl;
    std::cerr << "                      line-wise. Alternatively use --binary" << std::endl;
    std::cerr << "    --seed=[<value>]; random seed" << std::endl;
    std::cerr << std::endl;
    std::cerr << "csv options" << std::endl;
    std::cerr << comma::csv::options::usage( verbose ) << std::endl;
    std::cerr << "examples" << std::endl;
    std::cerr << "    output 70% of data:          cat full.csv | csv-thin 0.7" << std::endl;
    std::cerr << "    output once every 2 seconds: cat full.csv | csv-thin --period 2" << std::endl;
    std::cerr << "    using timestamp from input:  cat full.csv | csv-thin --period 2 --fields t" << std::endl;
    std::cerr << "    binary data:                 cat full.bin | csv-thin 0.1 --binary 3d" << std::endl;
    std::cerr << std::endl;
    exit( 0 );
}

static double rate;
static bool deterministic;
static bool invert;
static boost::optional< comma::uint32 > seed;
static boost::optional< boost::posix_time::microseconds > period;

struct timestamped
{
    boost::posix_time::ptime timestamp;
    timestamped() {}
    timestamped( const boost::posix_time::ptime& timestamp ) : timestamp( timestamp ) {}
};

namespace comma { namespace visiting {

template <> struct traits< timestamped >
{
    template < typename K, typename V > static void visit( const K&, const timestamped& p, V& v ) { v.apply( "t", p.timestamp ); }
    template < typename K, typename V > static void visit( const K&, timestamped& p, V& v ) { v.apply( "t", p.timestamp ); }
};

} } // namespace comma { namespace visiting {

static bool skip()
{
    if( period )
    {
        static boost::posix_time::ptime next_time = boost::posix_time::microsec_clock::universal_time();
        boost::posix_time::ptime now = boost::posix_time::microsec_clock::universal_time();
        if( now <= next_time ) { return true; }
        next_time += *period;
        return false;
    }
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
        
        static const unsigned long long size = 1.0e+9;
        static unsigned long long step = 0;
        static unsigned long long count = 0;
        ++count;
        if( count < ( step + 1 ) / rate ) { return true; }
        ++step;
        if( step == size ) { count = step = 0; }
        return false;
    }
    static boost::mt19937 rng = seed ? boost::mt19937( *seed ) : boost::mt19937();
    static boost::uniform_real<> dist( 0, 1 );
    static boost::variate_generator< boost::mt19937&, boost::uniform_real<> > random( rng, dist );
    static bool do_ignore = comma::math::less( rate, 1.0 );
    return do_ignore && random() > rate;
}

static bool keep() { return skip() == invert; }

static bool skip_by_timestamp( boost::posix_time::ptime timestamp )
{
    static boost::posix_time::ptime next_time = timestamp;
    if( timestamp < next_time ) { return true; }
    while( next_time <= timestamp ) { next_time += *period; }
    return false;
}

int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av, usage );
        bool binary = options.exists( "--size,-s,--binary,-b" );
        bool flush = options.exists( "--flush" );
        deterministic = options.exists( "--deterministic,-d" );
        invert = options.exists( "--invert,-i" );
        seed = options.optional< comma::uint32 >( "--seed" );
        if( options.exists( "--period" )) { period = boost::posix_time::microseconds( static_cast< unsigned int >( options.value< double >( "--period" ) * 1000000 ) ); }
        #ifdef WIN32
        if( binary ) { _setmode( _fileno( stdin ), _O_BINARY ); _setmode( _fileno( stdout ), _O_BINARY ); }
        #endif
        if( options.exists( "--fields" ))
        {
            if( !period ) { comma::say() << "--fields requires --period option" << std::endl; }
            comma::csv::input_stream< timestamped > istream( std::cin, comma::csv::options( options ) );
            while( std::cin.good() && !std::cin.eof() )
            {
                const timestamped* p = istream.read();
                if( !p ) { break; }
                if( skip_by_timestamp( p->timestamp ) != invert ) { continue; }
                if( istream.is_binary())
                {
                    std::cout.write( istream.binary().last(), istream.binary().size() );
                    if( flush ) { std::cout.flush(); }
                }
                else { std::cout << comma::join( istream.ascii().last(), istream.ascii().ascii().delimiter() ) << std::endl; }
            }
            return 0;
        }
        std::vector< std::string > v;
        if( !period )
        {
            v = options.unnamed( "--deterministic,-d", "-.*" );
            if( v.empty() ) { comma::say() << "please specify rate" << std::endl; return 1; }
            rate = boost::lexical_cast< double >( v[0] );
            if( comma::math::less( rate, 0 ) || comma::math::less( 1, rate ) ) { comma::say() << "expected rate between 0 and 1, got " << rate << std::endl; return 1; }
        }
        if( binary ) // quick and dirty, improve performance by reading larger buffer
        {
            std::size_t size = options.value( "--size,-s", 0u );
            std::string format_string = options.value< std::string >( "--binary,-b", "" );
            boost::optional< comma::csv::format > f;
            if( !format_string.empty() ) { f.reset( comma::csv::format( format_string ) ); }
            if( !size ) { size = f->size(); }
            if( f && f->size() != size ) { comma::say() << "expected consistent size, got --size " << size << " and --binary of size " << f->size() << std::endl; return 1; }
            unsigned int factor = 65536 / size; // arbitrary
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
                if( std::cin.gcount() < int( size ) ) { comma::say() << "expected " << size << " bytes; got only " << std::cin.gcount() << std::endl; return 1; }
                if( keep() ) { std::cout.write( &buf[0], size ); std::cout.flush(); }
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
                    if( offset != 0 ) { comma::say() << "expected at least " << size << " bytes, got only " << offset << std::endl; return 1; }
                    break;
                }
                offset += count;
                capacity -= count;
                for( ; offset >= size; cur += size, offset -= size )
                {
                    if( keep() ) { std::cout.write( cur, size ); }
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
                if( !line.empty() && keep() ) { std::cout << line << std::endl; }
            }
        }
        return 0;
    }
    catch( std::exception& ex ) { comma::say() << ex.what() << std::endl; }
    catch( ... ) { comma::say() << "unknown exception" << std::endl; }
    return 1;
}
