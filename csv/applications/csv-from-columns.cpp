#include <iostream>
#include <boost/lexical_cast.hpp>
#include <comma/application/command_line_options.h>

static void usage()
{
    std::cerr << std::endl;
    std::cerr << "take fixed-column format, output csv" << std::endl;
    std::cerr << "trailing whitespaces will be removed" << std::endl;
    std::cerr << "(since some systems still use fixed-column files)" << std::endl;
    std::cerr << std::endl;
    std::cerr << "usage: cat fixed-column.txt | csv-from-columns <sizes> [<options>] > fixed-columns.csv" << std::endl;
    std::cerr << std::endl;
    std::cerr << "sizes: field sizes in bytes, e.g. \"8,4,4\"" << std::endl;
    std::cerr << "       shortcuts: \"8,3*4\" is the same as \"8,4,4,4\"" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options" << std::endl;
    std::cerr << "    --delimiter,-d=<delimiter>: output delimiter; default: \",\"" << std::endl;
    std::cerr << std::endl;
    exit( 1 );
}

int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av );
        if( options.exists( "--help,-h" ) ) { usage(); }
        char delimiter = options.value( "--delimiter,-d", ',' );
        std::vector< std::string > unnamed = options.unnamed();
        if( unnamed.empty() ) { std::cerr << "csv-from-columns: expected column sizes, got none" << std::endl; return 1; }
        std::vector< unsigned int > sizes;
        std::vector< std::string > v = comma::split( unnamed[0], ',' );
        for( std::size_t i = 0; i < v.size(); ++i )
        {
            std::vector< std::string > w = comma::split( v[i], '*' );
            if( w[0].empty() ) { std::cerr << "csv-from-columns: invalid column size format: " << unnamed[0] << std::endl; return 1; }
            switch( w.size() )
            {
                case 1:
                    sizes.push_back( boost::lexical_cast< unsigned int >( w[0] ) );
                    break;
                case 2:
                    sizes.resize( sizes.size() + boost::lexical_cast< unsigned int >( w[0] ), boost::lexical_cast< unsigned int >( w[1] ) );
                    break;
                default:
                    std::cerr << "csv-from-columns: invalid column size format: " << unnamed[0] << std::endl;
                    return 1;
            }
        }
        while( std::cin.good() && !std::cin.eof() )
        {
            std::string line;
            std::getline( std::cin, line );
            if( line.empty() ) { break; }
            std::size_t size = line.size() - 1; // eat end of line
            if( line.empty() ) { continue; }
            const char* it = &line[0];
            const char* end = &line[0] + size;
            std::string d;
            for( std::size_t i = 0; i < sizes.size(); it += sizes[i], ++i )
            {
                std::cout << d;
                if( it < end )
                {
                    std::string s( it, ( it + sizes[i] ) > end ? end - it : sizes[i] );
                    std::string::size_type first = s.find_first_not_of( ' ' );
                    std::string::size_type last = s.find_last_not_of( ' ' );
                    if( last != std::string::npos ) { std::cout.write( it + first, last - first + 1 ); }
                }
                d = delimiter;
            }
            std::cout << std::endl;
        }
        return 0;
    }
    catch( std::exception& ex )
    {
        std::cerr << "csv-from-columns: " << ex.what() << std::endl;
    }
    catch( ... )
    {
        std::cerr << "csv-from-columns: unknown exception" << std::endl;
    }
    return 1;
}
