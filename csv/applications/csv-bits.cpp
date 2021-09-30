#include <array>
#include <memory>
#include <numeric>
#include <vector>
#include "../../application/command_line_options.h"
#include "../../base/exception.h"
#include "../../base/types.h"
#include "../../string/string.h"
#include "../stream.h"
#include "../traits.h"

void usage( bool )
{
    std::cerr << "converting between bits and csv and other bit operations" << std::endl;
    std::cerr << std::endl;
    std::cerr << "usage: cat input.bin | csv-bits <operation> <options> > output.bin" << std::endl;
    std::cerr << std::endl;
    std::cerr << "operations: from-csv (unpack), to-csv (pack)" << std::endl;
    std::cerr << std::endl;
    std::cerr << "operations" << std::endl;
    std::cerr << "    from-csv (pack): todo; convert csv to packed bits" << std::endl;
    std::cerr << "        options" << std::endl;
    std::cerr << "            --endian=<which>; default=big; endianness of input: big or little" << std::endl;
    std::cerr << "            --sizes=<sizes>; comma-separated bit field sizes (todo: support multiplier)" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    to-csv (unpack): todo; convert packed bits to csv" << std::endl;
    std::cerr << "        options" << std::endl;
    std::cerr << "            --endian=<which>; default=big; endianness of output: big or little" << std::endl;
    std::cerr << "            --sizes=<sizes>; comma-separated bit field sizes (todo: support multiplier)" << std::endl;
    std::cerr << std::endl;
    exit( 0 );
}

// todo
//   - document on gitlab
//   - to-csv
//     - unit test
//     - support uint64
//   - from-csv
//   - use constexpr

namespace comma { namespace csv_bits {

struct unpacked
{
    std::vector< comma::uint32 > values;
    unpacked( unsigned int size = 0 ): values( size, 0 ) {}
};

struct field
{
    unsigned int begin;
    unsigned int begin_byte;
    unsigned char begin_mask;
    unsigned int size;
    unsigned int bytes;
    unsigned int shift;
    bool little_endian;

    static std::array< unsigned char, 8 > begin_masks;

    field() = default;
    field( unsigned int begin, unsigned int size, bool little_endian )
        : begin( begin )
        , begin_byte( begin / 8 )
        , begin_mask( begin_masks[ begin % 8 ] )
        , size( size )
        , bytes( size / 8 + int( size % 8 > 0 ) )
        , shift( 64 - begin % 8 - size )
        , little_endian( little_endian )
    {
        if( size > sizeof( comma::uint32 ) * 8 ) { COMMA_THROW( comma::exception, "expected size up to " << ( sizeof( comma::uint32 ) * 8 ) << " bits; got: " << size ); }
        //std::cerr << "--> a: begin: " << begin << " begin byte: " << begin_byte << " bytes: " << bytes << " size: " << size << " begin mask: " << ( unsigned int )begin_mask << " shift: " << shift << std::endl;
    }

    comma::uint32 get( const std::vector< char >& buf ) const // todo: quick and dirty, watch performance
    {
        comma::uint64 r = 0;
        char* p = reinterpret_cast< char* >( &r );
        std::memcpy( p, &buf[ begin_byte ], bytes );
        //std::cerr << "--> b: p[0]: " << int( p[0] ) << " r: " << r << std::endl;
        p[0] &= begin_mask;
        //std::cerr << "--> c: p[0]: " << int( p[0] ) << " r: " << r << std::endl;
        r >>= shift;
        //std::cerr << "--> d: r: " << r << std::endl;
        if( !little_endian )
        {
            // todo
        }
        return r;
    }
};

std::array< unsigned char, 8 > field::begin_masks = { 255, 127, 63, 31, 15, 7, 3, 1 }; // todo: use constexpr

} } // namespace comma { namespace csv_bits {

namespace comma { namespace visiting {

template <> struct traits< comma::csv_bits::unpacked >
{
    template < typename K, typename V > static void visit( const K&, const comma::csv_bits::unpacked& p, V& v ) { v.apply( "values", p.values ); }
    template < typename K, typename V > static void visit( const K&, comma::csv_bits::unpacked& p, V& v ) { v.apply( "values", p.values ); }
};

} } // namespace comma { namespace visiting {

namespace comma { namespace csv_bits { namespace from_csv {

int run( const comma::command_line_options& options )
{
    #ifdef WIN32
        _setmode( _fileno( stdout ), _O_BINARY );
    #endif
    comma::csv::options csv( options );
    std::cerr << "csv-bits: from-csv: todo" << std::endl;
    return 1;
}

} } } // namespace comma { namespace csv_bits { namespace from_csv {

namespace comma { namespace csv_bits { namespace to_csv {

int run( const comma::command_line_options& options )
{
    #ifdef WIN32
        _setmode( _fileno( stdin ), _O_BINARY );
    #endif
    comma::csv::options csv( options );
    if( !csv.flush ) { std::cin.tie( NULL ); }
    const auto& sizes = comma::split_as< unsigned int >( options.value< std::string >( "--sizes" ), ',' );
    unsigned int size = std::accumulate( sizes.begin(), sizes.end(), 0 );
    if( size % 8 > 0 ) { std::cerr << "csv-bits: to-csv: expected input record size in bits divisible by 8; got: " << size << " (oddly-sized record support: todo)" << std::endl; return 1; }
    size /= 8;
    bool little_endian = options.value< std::string >( "--endian", "big" ) == "little";
    std::vector< std::pair< unsigned int, unsigned int > > indices;
    std::vector< comma::csv_bits::field > fields;
    unsigned int begin = 0;
    for( auto s: sizes ) { fields.push_back( comma::csv_bits::field( begin, s, little_endian ) ); begin += s; }
    std::vector< char > buf( size );
    comma::csv_bits::unpacked output( sizes.size() );
    comma::csv::output_stream< comma::csv_bits::unpacked > os( std::cout, csv, output );
    while( std::cin.good() )
    {
        std::cin.read( &buf[0], size );
        if( std::cin.gcount() <= 0 ) { break; }
        if( std::cin.gcount() < size ) { std::cerr << "csv-bits: to-csv: expected " << size << " byte(s); got: " << std::cin.gcount() << std::endl; return 1; }
        for( unsigned int i = 0; i < sizes.size(); ++i ) { output.values[i] = fields[i].get( buf ); }
        os.write( output );
    }
    return 0;
}

} } } // namespace comma { namespace csv_bits { namespace to_csv {

int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av, usage );
        const auto& unnamed = options.unnamed( "--flush, --verbose, -v", "-.*" );
        if( unnamed.empty() ) { std::cerr << "csv-bits: please specify operation" << std::endl; return 1; }
        if( unnamed.size() > 1 ) { std::cerr << "csv-bits: expected operation; got: " << comma::join( unnamed, ',' ) << std::endl; return 1; }
        const std::string& operation = unnamed[0];
        if( operation == "to-csv" || operation == "unpack" ) { return comma::csv_bits::to_csv::run( options ); }
        if( operation == "from-csv" || operation == "pack" ) { return comma::csv_bits::from_csv::run( options ); }
        std::cerr << "csv-bits: expected operation; got: \"" << operation << "\"" << std::endl;
    }
    catch( std::exception& ex ) { std::cerr << "csv-bits: " << ex.what() << std::endl; }
    catch( ... ) { std::cerr << "csv-bits: unknown exception" << std::endl; }
    return 1;
}
