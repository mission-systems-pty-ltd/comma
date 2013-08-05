#include <iostream>
#include <vector>
#include <comma/application/command_line_options.h>
#include <comma/csv/options.h>
#include <comma/string/string.h>

static void usage( bool verbose )
{
    std::cerr << "perform operations on csv columns" << std::endl;
    std::cerr << std::endl;
    std::cerr << "usage: cat data.csv | csv-shuffle <options> > shuffled.csv" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options" << std::endl;
    std::cerr << "    --help,-h: help; --help --verbose: more help" << std::endl;
    std::cerr << "    --fields,-f <fields>: input fields" << std::endl;
    std::cerr << "    --output-fields,--output,-o <fields>: output fields" << std::endl;
    std::cerr << "    --verbose,-v: more output" << std::endl;
    if( verbose ) { std::cerr << std::endl << comma::csv::options::usage() << std::endl; }
    std::cerr << std::endl;
    std::cerr << "examples" << std::endl;
    std::cerr << "    operations (for now): append, remove, swap" << std::endl;
    std::cerr << "    semantics:" << std::endl;
    std::cerr << "        remove:" << std::endl;
    std::cerr << "            cat xyz.csv | csv-shuffle --fields=x,y,z --output-fields=x,z" << std::endl;
    std::cerr << "        append:" << std::endl;
    std::cerr << "            cat xyz.csv | csv-shuffle --fields=x,y,z --output-fields=x,y,z,x" << std::endl;
    std::cerr << "        swap:" << std::endl;
    std::cerr << "            cat xyz.csv | csv-shuffle --fields=x,y,z --output-fields=y,z,x" << std::endl;
    std::cerr << "        remove x, swap y,z, append z two times:" << std::endl;
    std::cerr << "            cat xyz.csv | csv-shuffle --fields=x,y,z --output-fields=z,y,z,z" << std::endl;
    std::cerr << std::endl;
    exit( 1 );
}

struct field
{
    std::string name;
    unsigned int index;
    boost::optional< unsigned int > input_index;
    unsigned int size;
    field( const std::string& name, unsigned int index, unsigned int size ) : name( name ), index( index ), size( size ) {}
};

int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av );
        bool verbose = options.exists( "--verbose,-v" );
        if( options.exists( "--help,-h" ) ) { usage( verbose ); }
        comma::csv::options csv( options );
        std::vector< std::string > input_fields = comma::split( csv.fields, ',' );
        std::vector< std::string > output_fields = comma::split( options.value< std::string >( "--output-fields,--output,-o" ), ',' );
        std::vector< field > fields;
        for( unsigned int i = 0; i < output_fields.size(); ++i )
        {
            if( output_fields[i].empty() ) { continue; }
            fields.push_back( field( output_fields[i], i, csv.binary() ? csv.format().offset( i ).size : 0 ) );
        }
        if( fields.empty() ) { std::cerr << "csv-shuffle: please define at least one output field" << std::endl; return 1; }
        for( unsigned int i = 0; i < input_fields.size(); ++i )
        {
            for( unsigned int j = 0; j < fields.size(); ++j )
            {
                if( fields[j].name == input_fields[i] ) { fields[i].input_index = j; }
            }
        }
        for( unsigned int i = 0; i < output_fields.size(); ++i )
        {
            if( !fields[i].input_index ) { std::cerr << "csv-shuffle: \"" << fields[i].name << "\" not found in input fields" << std::endl; return 1; }
        }
        if( csv.binary() )
        {
            #ifdef WIN32
            _setmode( _fileno( stdin ), _O_BINARY );
            _setmode( _fileno( stdout ), _O_BINARY );
            #endif

            std::cerr << "csv-shuffle: binary: todo" << std::endl; return 1;
        }
        else
        {
            while( std::cin.good() && !std::cin.eof() )
            {
                std::string line;
                std::getline( std::cin, line );
                if( !line.empty() && *line.rbegin() == '\r' ) { line = line.substr( 0, line.length() - 1 ); } // windows... sigh...
                if( line.empty() ) { continue; }
                std::vector< std::string > v = comma::split( line, csv.delimiter );
                std::string delimiter;
                unsigned int previous_index = 0;
                for( unsigned int i = 0; i < fields.size(); ++i ) // quick and dirty
                {
                    for( unsigned int k = previous_index; k < fields[i].index && k < v.size(); ++k )
                    {
                        std::cout << csv.delimiter << v[k];
                        delimiter = csv.delimiter;
                    }
                    std::cout << csv.delimiter << v[ *fields[i].input_index ];
                    delimiter = csv.delimiter;
                }
                std::cout << std::endl;
            }
        }
        return 0;
    }
    catch( std::exception& ex )
    {
        std::cerr << "csv-shuffle: " << ex.what() << std::endl;
    }
    catch( ... )
    {
        std::cerr << "csv-shuffle: unknown exception" << std::endl;
    }
    return 1;
}
