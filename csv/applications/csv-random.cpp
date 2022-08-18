// Copyright (c) 2018 Vsevolod Vlaskine

/// @authors vsevolod vlaskine, kent hu

#include <algorithm>
#include <cstring>
#include <deque>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "../../application/command_line_options.h"
#include "../../base/exception.h"
#include "../../base/types.h"
#include "../../csv/stream.h"
#include "../../string/string.h"

static void usage( bool verbose )
{
    std::cerr << std::endl;
    std::cerr << "random operations on input stream" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options" << std::endl;
    std::cerr << "    --seed=[<unsigned int>]; random seed" << std::endl;
    std::cerr << std::endl;
    std::cerr << "operations" << std::endl;
    std::cerr << "    make: output pseudo-random numbers" << std::endl;
    std::cerr << std::endl;
    std::cerr << "        usage: csv-random make [<options>] > random.csv" << std::endl;
    std::cerr << "               cat records.csv | csv-random make --append [<options>] > appended.csv" << std::endl;
    std::cerr << std::endl;
    std::cerr << "        options" << std::endl;
    std::cerr << "            --append; append random numbers to stdin input" << std::endl;
    std::cerr << "            --distribution=<distribution>[;<options>]; default=uniform; values: uniform, gaussian, normal; todo: more distributions to plug in, just ask" << std::endl;
    std::cerr << "                uniform[;<min>;<max>]: if <min>, <max> not present, --range values will be used" << std::endl;
    std::cerr << "                gaussian[;<mean>;<sigma>]" << std::endl;
    std::cerr << "                normal: alias for gaussian" << std::endl;
    std::cerr << "            --engine=<engine>; default=mt19937_64; supported values: minstd_rand0, minstd_rand, mt19937, mt19937_64, ranlux24_base, ranlux48_base, ranlux24, ranlux48, knuth_b, default_random_engine" << std::endl;
    std::cerr << "            --output-binary; output random numbers as binary, or specify --binary=<format> for stdin input" << std::endl;
    std::cerr << "            --range=[<min>,<max>]; desired value range, default: whatever stl defines (usually numeric limits)" << std::endl;
    std::cerr << "            --type=<type>; default=ui; supported values: b, ub, w, uw, i, ui, l, ul, f, d; can have more than one <type> i.e. 3ui" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    true-random: output non-deterministic uniformly distributed unsigned int random numbers (if non-deterministic source is not available" << std::endl;
    std::cerr << "                 e.g. a hardware device, output will be pseudo-random" << std::endl;
    std::cerr << std::endl;
    std::cerr << "        usage: csv-random true-random [<options>]" << std::endl;
    std::cerr << "               cat records.csv | csv-random true-random --append <options> > appended.csv" << std::endl;
    std::cerr << std::endl;
    std::cerr << "        options" << std::endl;
    std::cerr << "            --append; append random number to stdin input" << std::endl;
    std::cerr << "            --once; output random number only once" << std::endl;
    std::cerr << "            --output-binary; output random numbers as binary, or specify --binary=<format> for stdin input" << std::endl;
    std::cerr << "            --type=<type>; default=ui; todo: supported values: ui; e.g: --type=3ui; --type=ui,ui,ui; etc" << std::endl;
    std::cerr << std::endl;
    std::cerr << "        example" << std::endl;
    std::cerr << "            > csv-random make --seed=$( csv-random true-random --once )" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    sample: output a uniformly distributed sample of input records of a given size; record order preserved" << std::endl;
    std::cerr << "            limitation: current implementation accumulates input records before outputting" << std::endl;
    std::cerr << "                        if records are large, it may be memory-inefficient; can be improved, just ask" << std::endl;
    std::cerr << std::endl;
    std::cerr << "        usage: cat records.csv | csv-random sample [<options>] > sample.csv" << std::endl;
    std::cerr << std::endl;
    std::cerr << "        options" << std::endl;
    std::cerr << "            --engine=<engine>; default=mt19937_64; supported values: minstd_rand0, minstd_rand, mt19937, mt19937_64, ranlux24_base, ranlux48_base, ranlux24, ranlux48, knuth_b, default_random_engine" << std::endl;
    std::cerr << "            --fields=[<fields>]; if 'block' field present sample each block, otherwise read whole input and then sample" << std::endl;
    std::cerr << "            --ratio=[<ratio>]; portion of each block to output, if block is too small, nothing will be output for it" << std::endl;
    std::cerr << "            --size=<n>; default=1; number of records to output in each block, if smaller than block size, output the whole block" << std::endl;
    std::cerr << "            --sliding-window,--window=[<size>]; todo: sample on sliding window of <size> records" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    shuffle: output input records in pseudo-random order" << std::endl;
    std::cerr << std::endl;
    std::cerr << "        usage: cat records.csv | csv-random shuffle [<options>] > shuffled.csv" << std::endl;
    std::cerr << std::endl;
    std::cerr << "        options" << std::endl;
    std::cerr << "            --engine=<engine>; default=mt19937_64; supported values: minstd_rand0, minstd_rand, mt19937, mt19937_64, ranlux24_base, ranlux48_base, ranlux24, ranlux48, knuth_b, default_random_engine" << std::endl;
    std::cerr << "            --fields=[<fields>]; if 'block' field present, shuffle each block, otherwise read whole input and then shuffle" << std::endl;
    std::cerr << "            --ratio=[<ratio>]; portion of each block to output, same as for \"sample\" operation, but shuffled" << std::endl;
    std::cerr << "            --size=<n>; default=1; number of records to output in each block, same as for \"sample\" operation, but shuffled" << std::endl;
    std::cerr << "            --sliding-window,--window=[<size>]; todo: shuffle on sliding window of <size> records" << std::endl;
    std::cerr << std::endl;
    std::cerr << "csv options:" << std::endl;
    std::cerr << comma::csv::options::usage( "", verbose ) << std::endl;
    std::cerr << std::endl;
    exit( 0 );
}

static bool verbose;
static comma::csv::options csv;
static boost::optional< int > seed;

namespace comma { namespace applications { namespace random { namespace shuffle {

struct input
{
    comma::uint32 block;
    input() : block( 0 ) {}
};

} } } } // namespace comma { namespace applications { namespace random { namespace shuffle {

namespace comma { namespace visiting {

template <> struct traits< comma::applications::random::shuffle::input >
{
    template < typename K, typename V > static void visit( const K&, const comma::applications::random::shuffle::input& p, V& v ) { v.apply( "block", p.block ); }
    template < typename K, typename V > static void visit( const K&, comma::applications::random::shuffle::input& p, V& v ) { v.apply( "block", p.block ); }
};

} } // namespace comma { namespace visiting {

namespace comma { namespace applications { namespace random {

template < typename T > struct type_traits { static T cast( const T t ) { return t; } };
template <> struct type_traits< char > { static int cast( const char t ) { return static_cast< int >( t ); } };
template <> struct type_traits< unsigned char > { static unsigned int cast( const unsigned char t ) { return static_cast< int >( t ); } };

namespace make {

template < typename T, template < typename > class Distribution, typename Engine >
static int run_impl( Distribution< T >& distribution, bool append, bool binary, std::size_t count, const boost::optional< std::pair< T, T > >& range )
{
    Engine engine = ::seed ? Engine( *::seed ) : Engine();
    if( !::csv.flush ) { std::cin.tie( nullptr ); }
    if( append )
    {
        if( ::csv.binary() )
        {
            std::vector< char > buf( ::csv.format().size() );
            while( std::cin.good() )
            {
                std::cin.read( &buf[0], buf.size() );
                if( std::cin.gcount() == 0 ) { break; }
                if( std::cin.gcount() != static_cast< int >( buf.size() ) ) { std::cerr << "csv-random make: expected " << buf.size() << " bytes; got " << std::cin.gcount() << std::endl; return 1; }
                std::cout.write( &buf[0], buf.size() );
                for( std::size_t i = 0; i < count; ++i )
                {
                    T r = distribution( engine );
                    std::cout.write( reinterpret_cast< char* >( &r ), sizeof( T ) );
                }
                if( ::csv.flush ) { std::cout.flush(); }
            }
            return 0;
        }
        while( std::cin.good() )
        {
            std::string s;
            std::getline( std::cin, s );
            if( s.empty() ) { continue; }
            std::cout << s;
            for( std::size_t i = 0; i < count; ++i ) { std::cout << ::csv.delimiter << type_traits< T >::cast( distribution( engine ) ); }
            std::cout << std::endl;
            if( ::csv.flush ) { std::cout.flush(); }
        }
        return 0;
    }
    if( binary )
    {
        while( std::cout.good() )
        {
            for( std::size_t i = 0; i < count; ++i )
            {
                T r = distribution( engine );
                std::cout.write( reinterpret_cast< char* >( &r ), sizeof( T ) );
            }
            if( ::csv.flush ) { std::cout.flush(); }
        }
        return 0;
    }
    while( std::cout.good() )
    {
        std::string comma;
        for( std::size_t i = 0; i < count; ++i )
        {
            std::cout << comma << type_traits< T >::cast( distribution( engine ) );
            comma = ::csv.delimiter;
        }
        std::cout << std::endl;
        if( ::csv.flush ) { std::cout.flush(); } // todo? remove? std::endl flushes anyway?
    }
    return 0;
}

template < typename T > static std::vector< T > _as( const std::vector< std::string >& v, unsigned int begin ) // todo? move to library?
{
    std::vector< T > r( v.size() - begin );
    for( unsigned int i = begin; i < v.size(); r[ i - begin ] = boost::lexical_cast< T >( v[i] ), ++i );
    return r;
}

template < typename T, template < typename > class Distribution, unsigned int Size > struct distribution_traits { static Distribution< T > make( const std::vector< T >& p ); }; // quick and dirty
template < typename T, template < typename > class Distribution > struct distribution_traits< T, Distribution, 0 > { static Distribution< T > make( const std::vector< T >& p ) { return Distribution< T >(); } };
template < typename T, template < typename > class Distribution > struct distribution_traits< T, Distribution, 1 > { static Distribution< T > make( const std::vector< T >& p ) { return Distribution< T >( p[0] ); } };
template < typename T, template < typename > class Distribution > struct distribution_traits< T, Distribution, 2 > { static Distribution< T > make( const std::vector< T >& p ) { return Distribution< T >( p[0], p[1] ); } };

template < typename T, template < typename > class Distribution > static Distribution< T > make_distribution( const std::vector< std::string >& params ) // quick and dirty
{
    const auto& p = _as< T >( params, 1 );
    switch( p.size() ) // quick and dirty; does not scale, but for now just to make it working
    {
        case 0: return distribution_traits< T, Distribution, 0 >::make( p );
        case 1: return distribution_traits< T, Distribution, 1 >::make( p );
        case 2: return distribution_traits< T, Distribution, 2 >::make( p );
        default: COMMA_THROW( comma::exception, "distribution traits for " << p.size() << " parameters: not implemented; just ask" );
    }
}

template < typename T, template < typename > class Distribution >
static int run_impl( const std::vector< std::string >& params, const comma::command_line_options& options )
{
    const auto& append = options.exists( "--append" );
    const auto& binary = options.exists( "--output-binary" ) || ::csv.binary();
    const auto& engine = options.value< std::string >( "--engine", "mt19937_64" );
    const auto& count = comma::csv::format( options.value< std::string >( "--type", "ui" ) ).count();
    boost::optional< std::pair< T, T > > range;
    if( options.exists( "--range" ) ) { range = comma::csv::ascii< std::pair< T, T > >().get( options.value< std::string >( "--range" ) ); }
    Distribution< T > distribution = make_distribution< T, Distribution >( params );
    if( engine == "minstd_rand0" ) { return run_impl< T, Distribution, std::minstd_rand0 >( distribution, append, binary, count, range ); }
    if( engine == "minstd_rand" ) { return run_impl< T, Distribution, std::minstd_rand >( distribution, append, binary, count, range ); }
    if( engine == "mt19937" ) { return run_impl< T, Distribution, std::mt19937 >( distribution, append, binary, count, range ); }
    if( engine == "mt19937_64" ) { return run_impl< T, Distribution, std::mt19937_64 >( distribution, append, binary, count, range ); }
    if( engine == "ranlux24_base" ) { return run_impl< T, Distribution, std::ranlux24_base >( distribution, append, binary, count, range ); }
    if( engine == "ranlux48_base" ) { return run_impl< T, Distribution, std::ranlux48_base >( distribution, append, binary, count, range ); }
    if( engine == "ranlux24" ) { return run_impl< T, Distribution, std::ranlux24 >( distribution, append, binary, count, range ); }
    if( engine == "ranlux48" ) { return run_impl< T, Distribution, std::ranlux48 >( distribution, append, binary, count, range ); }
    if( engine == "knuth_b" ) { return run_impl< T, Distribution, std::knuth_b >( distribution, append, binary, count, range ); }
    if( engine == "default_random_engine" ) { return run_impl< T, Distribution, std::default_random_engine >( distribution, append, binary, count, range ); }
    std::cerr << "csv-random make: expected engine; got: '" << engine << "'" << std::endl;
    return 1;
}

static int run( const comma::command_line_options& options ) // quick and dirty
{
    auto params = comma::split( options.value< std::string >( "--distribution", "uniform" ), ',' );
    const std::string& distribution = params[0];
    const auto& format = comma::csv::format( options.value< std::string >( "--type", "ui" ) );
    if ( format.collapsed_string().find( ',' ) != std::string::npos ) { std::cerr << "csv-random make: --type must be homogeneous i.e. ui or 2ui or 3ui" << std::endl; return 1; }
    if( distribution == "uniform" )
    {
        if( options.exists( "--range" ) ) // super-quick and dirty to preserve backward compatibility
        {
            if( params.size() > 1 ) { std::cerr << "csv-random make: uniform: either use --range or uniform[,<min>,<max>], not both" << std::endl; return 1; }
            const auto& r = comma::split( options.value< std::string >( "--range" ), ',' );
            params = { "uniform", r[0], r[1] };
        }
        if( params.size() != 1 && params.size() != 3 ) { std::cerr << "csv-random make: uniform: expected uniform[,<min>,<max>]; got: \"" << options.value< std::string >( "--distribution" ) << "\"" << std::endl; return 1; }
        switch( format.offset( 0 ).type )
        {
            case csv::format::int8: return run_impl< char, std::uniform_int_distribution >( params, options );
            case csv::format::uint8: return run_impl< unsigned char, std::uniform_int_distribution >( params, options );
            case csv::format::int16: return run_impl< comma::int16, std::uniform_int_distribution >( params, options );
            case csv::format::uint16: return run_impl< comma::uint16, std::uniform_int_distribution >( params, options );
            case csv::format::int32: return run_impl< comma::int32, std::uniform_int_distribution >( params, options );
            case csv::format::uint32: return run_impl< comma::uint32, std::uniform_int_distribution >( params, options );
            case csv::format::int64: return run_impl< comma::int64, std::uniform_int_distribution >( params, options );
            case csv::format::uint64: return run_impl< comma::uint64, std::uniform_int_distribution >( params, options );
            case csv::format::float_t: return run_impl< float, std::uniform_real_distribution >( params, options );
            case csv::format::double_t: return run_impl< double, std::uniform_real_distribution >( params, options );
            default: std::cerr << "csv-random make: uniform distribution: expected type; got: '" << format.string() << "'" << std::endl; return 1;
        }
    }
    if( distribution == "gaussian" || distribution == "normal" )
    {
        if( params.size() != 1 && params.size() != 3 ) { std::cerr << "csv-random make: gaussian: expected gaussian[,<mean>,<sigma>]; got: \"" << options.value< std::string >( "--distribution" ) << "\"" << std::endl; return 1; }
        switch( format.offset( 0 ).type )
        {
            case csv::format::float_t: return run_impl< float, std::normal_distribution >( params, options );
            case csv::format::double_t: return run_impl< double, std::normal_distribution >( params, options );
            default: std::cerr << "csv-random make: normal distribution: expected floating point --type; got unsupported type: '" << format.string() << "'" << std::endl; return 1;
        }
    }
    std::cerr << "csv-random make: expected distribution; got: '" << distribution << "'" << std::endl;
    return 1;
}

} // namespace make {

namespace shuffle {

template < typename Engine > static int run_impl( const comma::command_line_options& options, bool sample = false )
{
    auto engine = ::seed ? Engine( *::seed ) : Engine();
    std::deque< std::string > records;
    std::vector< unsigned int > indices; // quick and dirty
    unsigned int size = options.value( "--size", 1 ); // quick and dirty
    auto ratio = options.optional< float >( "--ratio" ); // quick and dirty
    auto sliding_window = options.optional< unsigned int >( "--sliding-window,--window" );
    if( sliding_window ) { std::cerr << "csv-random shuffle: --sliding-window: todo" << std::endl; return 1; }
    if( sliding_window ) { std::cerr << "csv-random shuffle: expected either block field or --sliding-window; got both" << std::endl; return 1; }
    comma::csv::input_stream< input > is( std::cin, ::csv );
    comma::uint32 block = 0;
    while( is.ready() || std::cin.good() )
    {
        const input* p = is.read();
        if( !p || p->block != block )
        {
            std::uniform_int_distribution< int > distribution( 0, records.size() - 1 ); // quick and dirty
            indices.resize( records.size() ); // quick and dirty
            for( unsigned int i = 0; i < indices.size(); ++i ) { indices[i] = i; }
            std::random_shuffle( indices.begin(), indices.end(), [&]( int ) -> int { return distribution( engine ); } ); // quick and dirty, watch performance
            unsigned int s = sample ? ( ratio ? int( records.size() * *ratio ) : size ) : records.size();
            if( sample ) { std::sort( indices.begin(), indices.begin() + s ); } // quick and dirty
            for( unsigned int i = 0; i < s; ++i ) { std::cout.write( &records[indices[i]][0], records[indices[i]].size() ); }
            records.clear();
            if( ::csv.flush ) { std::cout.flush(); }
            if( p ) { block = p->block; }
        }
        if( !p ) { break; }
        if( ::csv.binary() )
        {
            records.emplace_back();
            records.back().resize( ::csv.format().size() );
            std::memcpy( &records.back()[0], is.binary().last(), ::csv.format().size() );
        }
        else
        {
            records.push_back( comma::join( is.ascii().last(), ::csv.delimiter ) + "\n" );
        }
    }
    return 0;
}

static int run( const comma::command_line_options& options, bool sample = false )
{
    const auto& engine = options.value< std::string >( "--engine", "mt19937_64" );
    if( engine == "minstd_rand0" ) { return run_impl< std::minstd_rand0 >( options, sample ); }
    if( engine == "minstd_rand" ) { return run_impl< std::minstd_rand >( options, sample ); }
    if( engine == "mt19937" ) { return run_impl< std::mt19937>( options, sample ); }
    if( engine == "mt19937_64" ) { return run_impl< std::mt19937_64 >( options, sample ); }
    if( engine == "ranlux24_base" ) { return run_impl< std::ranlux24_base >( options, sample ); }
    if( engine == "ranlux48_base" ) { return run_impl< std::ranlux48_base >( options, sample ); }
    if( engine == "ranlux24" ) { return run_impl< std::ranlux24 >( options, sample ); }
    if( engine == "ranlux48" ) { return run_impl< std::ranlux48 >( options, sample ); }
    if( engine == "knuth_b" ) { return run_impl< std::knuth_b >( options, sample ); }
    if( engine == "default_random_engine" ) { return run_impl< std::default_random_engine >( options, sample ); }
    std::cerr << "csv-random " << ( sample ? "sample" : "shuffle" ) << ": expected engine; got: '" << engine << "'" << std::endl;
    return 1;
}

} // namespace shuffle {

namespace sample {

static int run( const comma::command_line_options& options ) { return shuffle::run( options, true ); } // quick and relatively dirty for now

} // namespace sample {

namespace true_random {

template < typename T >
static int run_impl( const comma::command_line_options& options, std::size_t count )
{
    std::random_device rd;
    const bool binary = options.exists( "--output-binary" ) || ::csv.binary();
    const bool flush = options.exists( "--flush" ) || ::csv.flush;
    auto output_line_to_stdout = [&]( std::string&& initial_delimiter )
    {
        for( std::size_t i = 0; i < count; ++i )
        {
            const T r = rd();
            if( binary ) { std::cout.write( reinterpret_cast< const char* >( &r ), sizeof( T ) ); }
            else { std::cout << initial_delimiter << type_traits< T >::cast( r ); initial_delimiter = ::csv.delimiter; }
        }
        if( !binary ) { std::cout << std::endl; }
        if( flush ) { std::cout << std::flush; }
    };
    if( options.exists( "--append" ) )
    {
        while( std::cin.good() )
        {
            auto buf = ::csv.binary() ? std::string( ::csv.format().size(), {} ) : std::string{};
            if( ::csv.binary() )
            {
                std::cin.read( &buf[0], buf.size() );
                if( std::cin.gcount() == 0 ) { return 0; }
                if( std::cin.gcount() != static_cast< int >( buf.size() ) ) { std::cerr << "csv-random true-random: expected " << buf.size() << " bytes; got " << std::cin.gcount() << std::endl; return 1; }
            }
            else
            {
                std::getline( std::cin, buf );
                if( buf.empty() ) { continue; }
            }
            std::cout.write( &buf[0], buf.size() );
            output_line_to_stdout( { ::csv.delimiter } );
        }
    }
    else
    {
        while( std::cout.good() )
        {
            output_line_to_stdout( {} );
            if( options.exists( "--once" ) ) { break; }
        }
    }
    return 0;
}

static int run( const comma::command_line_options& options )
{
    const auto format = comma::csv::format( options.value< std::string >( "--type", "ui" ) );
    if( format.collapsed_string().find( ',' ) != std::string::npos ) { std::cerr << "csv-random true-random: --type must be homogeneous i.e. ui or 2ui or 3ui" << std::endl; return 1; }
    switch( format.offset( 0 ).type ) {
        case csv::format::int8: return run_impl< char >( options, format.count() );
        case csv::format::uint8: return run_impl< unsigned char >( options, format.count() );
        case csv::format::int16: return run_impl< comma::int16 >( options, format.count() );
        case csv::format::uint16: return run_impl< comma::uint16 >( options, format.count() );
        case csv::format::int32: return run_impl< comma::int32 >( options, format.count() );
        case csv::format::uint32: return run_impl< comma::uint32 >( options, format.count() );
        case csv::format::int64: return run_impl< comma::int64 >( options, format.count() );
        case csv::format::uint64: return run_impl< comma::uint64 >( options, format.count() );
        case csv::format::float_t: return run_impl< float >( options, format.count() );
        case csv::format::double_t: return run_impl< double >( options, format.count() );
        default: std::cerr << "csv-random true-random: expected type; got: '" << format.string() << "'" << std::endl;
    }
    return 1;
}

} // namespace true_random {

} } } // namespace comma { namespace applications { namespace random {

int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av, usage );
        const auto& unnamed = options.unnamed( "--append,--flush,--verbose,-v", "-.*" );
        if( unnamed.empty() ) { std::cerr << "csv-random: please specify operation" << std::endl; return 1; }
        ::csv = comma::csv::options( options );
        std::cout.precision( ::csv.precision );
        ::seed = options.optional< comma::uint32 >( "--seed" );
        ::verbose = options.exists( "--verbose,-v" );
        std::string operation = unnamed[0];
        if( operation == "make" ) { return comma::applications::random::make::run( options ); }
        if( operation == "sample" ) { return comma::applications::random::sample::run( options ); }
        if( operation == "shuffle" ) { return comma::applications::random::shuffle::run( options ); }
        if( operation == "true-random" ) { return comma::applications::random::true_random::run( options ); }
        std::cerr << "csv-random: expected operation; got: '" << operation << "'" << std::endl;
        return 1;
    }
    catch( std::exception& ex ) { std::cerr << "csv-random: " << ex.what() << std::endl; }
    catch( ... ) { std::cerr << "csv-random: unknown exception" << std::endl; }
    return 1;
}
