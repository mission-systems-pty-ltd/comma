#include <set>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <comma/application/command_line_options.h>
#include <comma/string/string.h>

static void usage( bool verbose )
{
    std::cerr << "take csv string, quote anything that is not a number" << std::endl;
    std::cerr << std::endl;
    std::cerr << "usage: cat lines.csv | csv-quote [<options>]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options" << std::endl;
    std::cerr << "    --delimiter,-d=<delimiter>; default: ," << std::endl;
    std::cerr << "    --fields=<fields>: quote given fields, even if their values are numbers" << std::endl;
    std::cerr << "                       if --unquote, unquote only given fields" << std::endl;
    std::cerr << "    --force=<fields>: quote given fields, if their values are numbers" << std::endl;
    std::cerr << "    --escaped: escape quotes with backslash" << std::endl;
    std::cerr << "    --quote=<quote sign>; default: double quote" << std::endl;
    std::cerr << "    --unquote; remove quotes" << std::endl;
    std::cerr << std::endl;
    exit( 1 );
}

int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av, usage );
        std::set< std::size_t > fields;
        {
            const std::vector< std::string >& v = comma::split( options.value< std::string >( "--fields", "" ), ',' );
            for( unsigned int i = 0; i < v.size(); ++i ) { if( !v[i].empty() ) { fields.insert( i ); } }
        }
        std::set< std::size_t > forced;
        {
            const std::vector< std::string >& v = comma::split( options.value< std::string >( "--force", "" ), ',' );
            for( unsigned int i = 0; i < v.size(); ++i ) { if( !v[i].empty() ) { forced.insert( i ); } }
        }
        char delimiter = options.value( "--delimiter,-d", ',' );
        char quote = options.value( "--quote", '\"' );
        bool unquote = options.exists( "--unquote" );
        std::string backslash;
        if( options.exists( "--escape" ) ) { backslash = "\\"; }
        while( std::cin.good() )
        {
            std::string line;
            std::getline( std::cin, line );
            if( line.empty() ) { continue; }
            const std::vector< std::string >& v = comma::split( line, delimiter );
            std::string comma;
            for( std::size_t i = 0; i < v.size(); ++i )
            {
                bool has_field = fields.empty() || fields.find( i ) != fields.end();
                std::cout << comma;
                comma = delimiter;
                if( unquote )
                {
                    std::cout << ( has_field ? comma::strip( v[i], quote ) : v[i] );
                }
                else
                {
                    const std::string& value = comma::strip( v[i], quote );
                    bool do_quote = false;
                    if( has_field )
                    {
                        do_quote = true;
                        if( forced.find( i ) == forced.end() )
                        {
                            try
                            {
                                boost::lexical_cast< double >( value );
                                do_quote = false;
                            }
                            catch( ... ) {}
                        }
                    }
                    if( do_quote ) { std::cout << backslash << quote << value << backslash << quote; }
                    else { std::cout << value; }
                }
            }
            std::cout << std::endl;
        }
        return 0;
    }
    catch( std::exception& ex ) { std::cerr << "csv-quote: " << ex.what() << std::endl; }
    catch( ... ) { std::cerr << "csv-quote: unknown exception" << std::endl; }
    return 1;
}
