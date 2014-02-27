#include <boost/date_time/posix_time/ptime.hpp>
#include <queue>
#include <deque>
#include <functional>
#include <boost/function.hpp>
#include <comma/csv/stream.h>
#include <comma/visiting/traits.h>
#include <comma/application/command_line_options.h>
#include <comma/name_value/ptree.h>

static const std::string& name() {
    static const std::string name = "aero-progress";
    return name;
}


namespace impl_ {
    

/// Input data structure
struct log {
    boost::posix_time::ptime timestamp;
    std::string name;
    bool is_begin;     // true for begin, false for end
};

struct elapsed_sum {
    std::deque< std::string > names;    // path names in order
    double duration;
};
    
} // namespace impl_ {
    
std::ostream& operator<<( std::ostream& os, const impl_::log& l ) { os << l.name; return os; }
    
namespace comma { namespace visiting {
    
template < > struct traits< impl_::log > {
    template< typename K, typename V > static void visit( const K& k, impl_::log& t, V& v )
    {
        v.apply( "timestamp", t.timestamp );
        v.apply( "name", t.name );
        std::string tmp;
        v.apply( "is_begin", tmp );
        t.is_begin = ( tmp == "begin" );
    }
    template< typename K, typename V > static void visit( const K& k, const impl_::log& t, V& v )
    {
        v.apply( "timestamp", t.timestamp );
        v.apply( "name", t.name );
        v.apply( "is_begin", t.is_begin ? std::string("begin") : std::string("end")  );
    }
};

    
} } // namespace comma { namespace visiting { 


static void usage( bool verbose=false )
{
    std::cerr << std::endl;
    std::cerr << "cat progress.csv | " << name() << " [<options>] > stat.csv" << std::endl;
    std::cerr << std::endl;
    std::cerr << "modes" << std::endl;
    std::cerr << "    These are mutually exclusive." << std::endl;
    std::cerr << "    <no option>: Outputs path value for input data: " << comma::join( comma::csv::names< impl_::log >(), ',' )  << std::endl;
    std::cerr << "                 Output format is 'path/{begin,end}=<ISO timestamp>'" << std::endl;
    std::cerr << "    --elapsed:   Outputs path value with 'elapsed' time, input data format: " << comma::join( comma::csv::names< impl_::log >(), ',' )  << std::endl;
    std::cerr << "                 Output format is 'path/elapsed=<duration in second>'" << std::endl;
    std::cerr << "    --sum:       Outputs path value with 'elapsed' time, taking input data from --elapsed mode." << std::endl;
    std::cerr << "                 Output format is 'path/elapsed=<duration in second>'" << std::endl;
    std::cerr << "                 Elapsed duration is duration sum of run with the same path e.g. plan-fuel/flight-prm called multiple times." << std::endl;
    std::cerr << "    --ratio [path]" << std::endl;
    std::cerr << "                 Outputs path value with 'elapsed' time and ratio of total time, taking input data from --sum or --elapsed mode." << std::endl;
    std::cerr << "                 Output format is 'path/elapsed=<duration in second>'" << std::endl;
    std::cerr << "                 Output format is 'path/ratio=< ratio to total time or time of [path] if given >'" << std::endl;
    std::cerr << "options" << std::endl;
    std::cerr << "    --help,-h: help; --help --verbose: more help" << std::endl;
    std::cerr << std::endl;
    exit( 1 );
}
static const std::string start = "begin";
static const std::string finished = "end";

void output( const impl_::log& begin, const impl_::log& end, const std::string& enclosing_branches )
{
    std::string addition_sep = enclosing_branches.empty() ? "" : "/";
    std::cout << enclosing_branches << addition_sep << begin.name << '/' << start     << '=' << boost::posix_time::to_iso_string( begin.timestamp ) << std::endl; 
    std::cout << enclosing_branches << addition_sep << end.name << '/'   << finished  << '=' << boost::posix_time::to_iso_string( end.timestamp ) << std::endl; 
}
void output_elapsed( const impl_::log& begin, const impl_::log& end, const std::string& enclosing_branches )
{
    std::string addition_sep = enclosing_branches.empty() ? "" : "/";
    std::cout << enclosing_branches << addition_sep << end.name << '/'   << "elapsed"  << '=' << ( (end.timestamp - begin.timestamp).total_milliseconds() / 1000.0 ) << std::endl; 
}

const impl_::log* get_log()
{
    static comma::csv::input_stream< impl_::log > istream( std::cin );
    return istream.read();
}

static char delimiter = ';';
static const char equal_sign = '=';

/// Merge values (expects double) with the same key
/// Returns the total value of all keys
double merge_elapsed( boost::property_tree::ptree& tree, std::set< std::string >& leaf_keys  )
{
    using boost::property_tree::ptree;
    
    std::string line;
    double total_elapsed = 0;
    while( std::cin.good() && !std::cin.eof() )
    {
        std::getline( std::cin, line );
        if( line.empty() ) continue;
        std::string::size_type p = line.find_first_of( equal_sign );
        if( p == std::string::npos ) { COMMA_THROW( comma::exception, "expected '" << delimiter << "'-separated xpath" << equal_sign << "value pairs; got \"" << line << "\"" ); }
        
        //const std::string key = comma::strip( v[i].substr( 0, p );
        const std::string key_str = comma::strip( line.substr( 0, p ), '"' );
        ptree::path_type key( key_str, '/' );
        double value = boost::lexical_cast< double >( comma::strip( line.substr( p + 1), '"' ) );
        total_elapsed += value;
        
        // Check if there is an existing value, if so merge elapsed time
        boost::optional< double > existing_value = tree.get_optional< double >( key );
        if( existing_value ) { value += *existing_value; } // sum it up
        
        leaf_keys.insert( key_str );
        tree.put( key, value );
    
    }
    
    return total_elapsed;
}


namespace impl_ {
    
template < typename T, typename L, typename O >
void process_begin_end( L get_log, O output )
{
//     static comma::csv::input_stream< T > istream( std::cin );
    static comma::csv::output_stream< T > estream( std::cerr );
    std::deque< T > stack; 
    while( std::cin.good() && !std::cin.eof() )
    {
        const T* plog = get_log();
        if( plog == NULL ) { break; }
        const T& message = *plog;
        
        
        if( stack.empty() || message.name == start ) {
            stack.push_back( message );
        }
        else
        {
            if( stack.empty() ) {
                std::cerr << name() << ": failed on "; estream.write( message );
                COMMA_THROW( comma::exception, "'end' must have a 'start' log entry" );
            }
            
            if( stack.back().name != message.name )
            {
                if( !message.is_begin ) 
                { 
                    std::cerr << name() << ": failed on "; estream.write( message );
                    COMMA_THROW( comma::exception, "incorrect nexting for log entry, expecting 'begin'" );
                }
                stack.push_back( message );
            }
            else     // must be an 'end' 
            {
                const T& begin = stack.back();
                output( begin, message, comma::join( stack, stack.size()-1, '/' ) );
                stack.pop_back();
            }
        }
    
    }
}

    
} // namespace impl_ {

int main( int ac, char** av )
{
    comma::command_line_options options( ac, av );
    
    if( options.exists( "-h|--help" ) ) { usage(); }
    
    try
    {
        if( options.exists( "--sum" ) )
        {
            
            boost::property_tree::ptree ptree; 
            std::set< std::string > leaf_keys;
            merge_elapsed( ptree, leaf_keys );
            comma::property_tree::to_path_value( std::cout, ptree, equal_sign, '\n' );
        }
        else if( options.exists( "--ratio" ) )
        {
            
            boost::property_tree::ptree ptree; 
            std::set< std::string > leaf_keys;
            double total_time = merge_elapsed( ptree, leaf_keys );
            
            // This is the optional path for base denominator
            std::vector< std::string > no_names = options.unnamed( "-h,--help,--elapsed,--sum,--ratio", "" );
            if( !no_names.empty() )
            {
//                 std::cerr << "no names: " << no_names.front() << std::endl;
                // This is the path for base denominator
                const boost::property_tree::ptree::path_type key( no_names.front(), '/' );
                boost::optional< double > denominator = ptree.get_optional< double >( key );
                
                if( !denominator ) { COMMA_THROW( comma::exception, "failed to find path in input data: " + no_names.front() ); }
                total_time = *denominator;
            }
            
//             std::cerr << "reached here" << std::endl;
            // Now make ratio tree
            for( const auto& key_str : leaf_keys )
            {
                const boost::property_tree::ptree::path_type key( key_str, '/' );
                std::string ratio_key_str =  key_str.substr(0, key_str.find_last_of( '/' ) );
                const boost::property_tree::ptree::path_type ratio_key( ratio_key_str + '/' + "ratio", '/' );
                
//                 std::cerr << "key_str: " << key_str << " ratio_key_str: " << ratio_key_str << '/' << "ratio" << std::endl;
                boost::optional< double > value = ptree.get_optional< double >( key );
                ptree.put( ratio_key, (*value)/total_time );
            }
            comma::property_tree::to_path_value( std::cout, ptree, equal_sign, '\n' );
        }
        else if( options.exists( "--elapsed" ) )
        {
            std::function< const impl_::log*() > extractor( &get_log );
            std::function< void( const impl_::log&, const impl_::log&, const std::string&) > outputting( &output_elapsed );
            impl_::process_begin_end< impl_::log >( extractor , outputting );
            
            return 0;
        }
        else
        {
            std::function< const impl_::log*() > extractor( &get_log );
            std::function< void( const impl_::log&, const impl_::log&, const std::string&) > outputting( &output );
            impl_::process_begin_end< impl_::log >( extractor , outputting );
            
            return 0;
        }
        
    }
    catch( std::exception& e ) {
        std::cerr << name() << ": exception caught - " << e.what() << std::endl;
    }
    catch(...) {
        std::cerr << name() << ": unknown exception caught, terminating." << std::endl;
    }
}
