#include <boost/date_time/posix_time/ptime.hpp>
#include <queue>
#include <deque>
#include <comma/csv/stream.h>
#include <comma/visiting/traits.h>
#include <comma/application/command_line_options.h>

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

comma::csv::output_stream< impl_::log >& estream()
{
    static comma::csv::output_stream< impl_::log > es( std::cerr );
    return es;
}
comma::csv::output_stream< impl_::log >& ostream() {
    static comma::csv::output_stream< impl_::log > os( std::cout );
    return os;
}

static void usage( bool verbose=false )
{
    std::cerr << std::endl;
    std::cerr << "cat progress.csv | " << name() << "  [<options>] > stat.csv" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options" << std::endl;
    std::cerr << "    --help,-h: help; --help --verbose: more help" << std::endl;
//     std::cerr << "    --make-unidirectional,-u: for bidirectional legs output one leg for each direction, each of type forward" << std::endl;
//     std::cerr << "                              for backward direction swap points and change type to forward" << std::endl;
//     std::cerr << "                              (\"airways,waypoints\" only)" << std::endl;
    if( verbose )
    {
        std::cerr << std::endl;
        std::cerr << "csv options" << std::endl;
        std::cerr << comma::csv::options::usage() << std::endl;
    }
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
    std::cout << enclosing_branches << addition_sep << end.name << '/'   << "elapsed" << '=' << ( double( ( end.timestamp - begin.timestamp ).total_milliseconds() ) / 1000 ) << std::endl; 
}

int main( int ac, char** av )
{
    comma::command_line_options options( ac, av );
    
    if( options.exists( "-h|--help" ) ) { usage(); }
    
    comma::csv::input_stream< impl_::log > istream( std::cin );
    
    
    try
    {
        std::deque< impl_::log > stack; 
        
        while( std::cin.good() && !std::cin.eof() )
        {
            const impl_::log* plog = istream.read();
            if( plog == NULL ) { break; }
            const impl_::log& message = *plog;
            
            
            if( stack.empty() || message.name == start ) {
                stack.push_back( message );
            }
            else
            {
                if( stack.empty() ) {
                    std::cerr << name() << ": failed on "; estream().write( message );
                    COMMA_THROW( comma::exception, "'end' must have a 'start' log entry" );
                }
                
                if( stack.back().name != message.name )
                {
                    if( !message.is_begin ) 
                    { 
                        std::cerr << name() << ": failed on "; estream().write( message );
                        COMMA_THROW( comma::exception, "incorrect nexting for log entry, expecting 'begin'" );
                    }
                    stack.push_back( message );
                }
                else     // must be an 'end' 
                {
                    const impl_::log& begin = stack.back();
                    output( begin, message, comma::join( stack, stack.size()-1, '/' ) );
                    stack.pop_back();
                }
            }
        
        }
        
    }
    catch( std::exception& e ) {
        std::cerr << name() << ": exception cause - " << e.what() << std::endl;
    }
    catch(...) {
        std::cerr << name() << ": unknown exception caught, terminating." << std::endl;
    }
}
