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

#include <boost/date_time/posix_time/ptime.hpp>
#include <vector>
#include <boost/unordered_map.hpp>
#include <deque>
#include <boost/unordered_set.hpp>
#include <functional>
#include <boost/functional.hpp>
#include <boost/math/special_functions/round.hpp>
#include "../../csv/stream.h"
#include "../../visiting/traits.h"
#include "../../application/command_line_options.h"
#include "../../name_value/ptree.h"

static const std::string& name() {
    static const std::string name = "comma-progress";
    return name;
}

static char delimiter = ';';
static const char equal_sign = '=';


namespace impl_ {
    

/// Input data structure
struct log {
    std::string name;
    bool is_begin;     // true for begin, false for end
    boost::posix_time::ptime timestamp;
};

// strangely, if this operator is put in global namespace, it does not compile on some versions of gcc
static std::ostream& operator<<( std::ostream& os, const impl_::log& l ) { os << l.name; return os; }

} // namespace impl_ {

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

static void usage( bool verbose )
{
    std::cerr << "\nsummarise timestamped elapsed time data";
    std::cerr << "\n";
    std::cerr << "\nusage: cat progress.csv | " << name() << " [<options>] > stat.csv";
    std::cerr << "\n";
    std::cerr << "\noptions:";
    std::cerr << "\n    --help,-h:     display this help message and exit";
    std::cerr << "\n    --verbose,-v:  more output";
    std::cerr << "\n    --elapsed:     output path value with elapsed time";
    std::cerr << "\n    --sum          output path value with summed elapsed time";
    std::cerr << "\n";
    std::cerr << "\nelapsed options:";
    std::cerr << "\n    --from-path-value,--from-pv: take input from output of <no option>";
    std::cerr << "\n";
    std::cerr << "\nsum options:";
    std::cerr << "\n    --count:          adds number of occurances of <path> (requires --mean)";
    std::cerr << "\n    --mean:           adds mean duration for duplicate paths";
    std::cerr << "\n    --percentage,-P:  express --ratio as a percentage";
    std::cerr << "\n    --ratio [<path>]: adds ratio to total time or time of [path] if given";
    std::cerr << "\n";
    std::cerr << "\nmodes:";
    std::cerr << "\n    with no option comma-progress takes " << comma::join( comma::csv::names< impl_::log >(), ',' ) << " and converts to";
    std::cerr << "\n    path-value format of 'path/{begin,end}=<ISO timestamp>'";
    std::cerr << "\n";
    std::cerr << "\n    --elapsed";
    std::cerr << "\n        input data format: " << comma::join( comma::csv::names< impl_::log >(), ',' );
    std::cerr << "\n        output format: 'path/elapsed=<duration in second>'";
    std::cerr << "\n        if --from-path-value is given input format is <no option> output";
    std::cerr << "\n";
    std::cerr << "\n    --sum";
    std::cerr << "\n        input: data in format from --elapsed mode";
    std::cerr << "\n        output: path-value with summed 'elapsed' time";
    std::cerr << "\n        elapsed duration sums of runs with the same <path>";
    std::cerr << "\n        additional accumlation stats can be added with --mean, --count, --ratio";
    std::cerr << "\n        options in the format <path>/<stat>=<value>";
    std::cerr << "\n";
    if( verbose )
    {
        std::cerr << "\nexamples:";
        std::cerr << "\n    --- create input data ---";
        std::cerr << "\n    cat <<-EOF > data.csv";
        std::cerr << "\n\t20230101T120000,main,begin";
        std::cerr << "\n\t20230101T120100,sub_a,begin";
        std::cerr << "\n\t20230101T120200,sub_a,end";
        std::cerr << "\n\t20230101T120200,sub_b,begin";
        std::cerr << "\n\t20230101T120600,sub_b,end";
        std::cerr << "\n\t20230101T120600,sub_a,begin";
        std::cerr << "\n\t20230101T120800,sub_a,end";
        std::cerr << "\n\t20230101T121000,main,end";
        std::cerr << "\n\tEOF";
        std::cerr << "\n";
        std::cerr << "\n    --- create path/value data ---";
        std::cerr << "\n    cat data.csv | comma-progress";
        std::cerr << "\n";
        std::cerr << "\n    --- calculate elapsed times ---";
        std::cerr << "\n    cat data.csv | comma-progress --elapsed";
        std::cerr << "\n    cat data.csv | comma-progress | comma-progress --elapsed --from-path-value";
        std::cerr << "\n";
        std::cerr << "\n    --- sum up repeated entries ---";
        std::cerr << "\n    cat data.csv | comma-progress --elapsed > elapsed.csv";
        std::cerr << "\n    cat elapsed.csv | comma-progress --sum";
        std::cerr << "\n    cat elapsed.csv | comma-progress --sum --mean";
        std::cerr << "\n    cat elapsed.csv | comma-progress --sum --mean --count";
        std::cerr << "\n    cat elapsed.csv | comma-progress --sum --ratio";
        std::cerr << "\n    cat elapsed.csv | comma-progress --sum --ratio --percentage";
        std::cerr << "\n    cat elapsed.csv | comma-progress --sum --ratio main/sub_b";
    }
    else
    {
        std::cerr << "\nsee comma-progress --help --verbose for examples";
    }
    std::cerr << "\n" << std::endl;
    exit( 1 );
}

static const std::string start = "begin";
static const std::string finished = "end";

// output begin, and end of a function/script in path value - nesting/tree - format
void output( const impl_::log& begin, const impl_::log& end, const std::string& enclosing_branches )
{
    std::string addition_sep = enclosing_branches.empty() ? "" : "/";
    std::cout << enclosing_branches << addition_sep << begin.name << '/' << start     << '=' << boost::posix_time::to_iso_string( begin.timestamp ) << std::endl; 
    std::cout << enclosing_branches << addition_sep << end.name << '/'   << finished  << '=' << boost::posix_time::to_iso_string( end.timestamp ) << std::endl; 
}
/// Output in path value with elapsed: end time - begin time
void output_elapsed( const impl_::log& begin, const impl_::log& end, const std::string& enclosing_branches )
{
    std::string addition_sep = enclosing_branches.empty() ? "" : "/";
    std::cout << enclosing_branches << addition_sep << end.name << '/'   << "elapsed"  << '=' << ( (end.timestamp - begin.timestamp).total_milliseconds() / 1000.0 ) << std::endl; 
}

// Retrieve log from std::cin using comma
const impl_::log* get_log()
{
    static comma::csv::input_stream< impl_::log > istream( std::cin );
    return istream.read();
}
// Retrieve log data from path value format, but it is the same data as get_log
const impl_::log* get_log_path_value()
{
    static impl_::log log;
    std::string line;
    std::getline( std::cin, line );
    if( line.empty() || line[0] == '#' ) return NULL;
        
    std::string::size_type p = line.find_first_of( equal_sign );
    if( p == std::string::npos ) { COMMA_THROW( comma::exception, "expected '" << delimiter << "'-separated xpath" << equal_sign << "value pairs; got \"" << line << "\"" ); }
    
    const std::string path = comma::strip( line.substr( 0, p ), '"' );
    try { log.timestamp = boost::posix_time::from_iso_string( comma::strip( line.substr( p + 1), '"' ) ); }
    catch( std::exception& e ) {
        std::cerr << name() << ": failed to parse date time from: " << line << std::endl;
        COMMA_THROW( comma::exception, std::string("failed to parse date time, cause: " ) + e.what() );
    }
    
    std::vector< std::string > names = comma::split( path, '/' );
    // TODO check for path
    if( names.empty() || names.back().empty() ) {
        std::cerr << name() << ": failed to parse path value line, cannot get path: '" << line << '\'' << std::endl;
        COMMA_THROW( comma::exception, "failed to parse: " + line );
    }
    log.name = comma::join( names, names.size() - 1, '/' );
    log.is_begin = (names.back()== start);
    
    return &log;
}

static const std::string elapsed_end("/elapsed");

/// Merge values (expects double) with the same key 
/// It merges values for identical key string that end in '/elapsed' and 
///  add a '/mean' key with the mean of the values 
/// All keys are returned via 'leaf_keys'
/// add_mean_key: adds '/mean' when sum combine '<path>/elapsed'
/// add_count_key: adds '/count' when sum combine '<path>/elapsed'
void merge_elapsed( boost::property_tree::ptree& tree, 
                    boost::unordered_set< std::string >& leaf_keys, 
                    bool add_mean_key, bool add_count_key )
{
    using boost::property_tree::ptree;
    
    std::string line;
    // counts the number of times a key is used for 'mean' value calculation
    boost::unordered_map< std::string, int > key_counts;    
    while( std::cin.good() && !std::cin.eof() )
    {
        std::getline( std::cin, line );
        if( line.empty() ) continue;
        std::string::size_type p = line.find_first_of( equal_sign );
        if( p == std::string::npos ) { COMMA_THROW( comma::exception, "expected '" << delimiter << "'-separated xpath" << equal_sign << "value pairs; got \"" << line << "\"" ); }
        
        //const std::string key = comma::strip( v[i].substr( 0, p );
        const std::string key_str = comma::strip( line.substr( 0, p ), '"' );
        ptree::path_type key( key_str, '/' );
        
        std::string value_str = comma::strip( line.substr( p + 1), '"' );
        
        // Only merges keys ending in '/elapsed'
        if( key_str.size() < elapsed_end.size() || key_str.substr( key_str.size() - elapsed_end.size() ) != elapsed_end ) { 
            tree.put( key, value_str );     // not 'elapsed' ending key, just put it into the ptree, may replaces earlier value for same key
        }
        else
        {   // This is /elapsed key, count occurances
            int& count = key_counts[ key_str ];
            ++count;
            
            // Get value and merge if needed
            
            double value = boost::lexical_cast< double >( value_str );
            
            // Check if there is an existing value, if so merge elapsed time
            boost::optional< double > existing_value = tree.get_optional< double >( key );
            if( existing_value ) { value += *existing_value; } // sum it up
            
            leaf_keys.insert( key_str );
            tree.put( key, value );
            
            // wether to create a 'mean' key - if there is more than one elapsed
            if( add_mean_key && count > 1 ) 
            {
                //make a 'mean' key by replacing /elapsed with /mean
                std::ostringstream ss;
                ss << key_str.substr( 0,  key_str.size() - elapsed_end.size() ) << "/mean";
                ptree::path_type mean_key( ss.str(), '/' );
                
                tree.put( mean_key, value/count );
                
                if( !add_count_key ) { continue; }
                std::ostringstream cc;
                cc << key_str.substr( 0,  key_str.size() - elapsed_end.size() ) << "/count";
                ptree::path_type count_key( cc.str(), '/' );
                tree.put( count_key, count );
            }
        }
        
    }
}


namespace impl_ {
    
/// Take input data in the form of impl_::logs, and convert them into path value structure
/// Where path is the tree/nested path
// Example input
//     20140226T162515.444639,app_a,begin
//     20140226T162515.450169,app_a,end
//     20140226T162515.485293,plan_a,begin
//     20140226T162515.553752,app_b,begin
//     20140226T162519.128530,app_b,end
//     20140226T162519.133253,app_c,begin
//     20140226T162519.170131,app_c,end
//     20140226T162519.279924,app_a,begin
//     20140226T162519.292069,app_a,end
//     20140226T162519.344238,app_b,begin
//     20140226T162522.012378,app_b,end
//     20140226T162522.160048,app_a,begin
//     20140226T162522.172084,app_a,end
//     20140226T162522.227812,plan_a,end
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
        
        
        if( stack.empty() ) {
            stack.push_back( message );
        }
        else
        {
            if( stack.empty() ) {
                std::cerr << name() << ": failed on "; estream.write( message );
                COMMA_THROW( comma::exception, "'end' must have a 'start' log entry" );
            }
            
            if( message.is_begin ) 
            {
                stack.push_back( message );
            }
            else 
            {
                if( stack.back().name != message.name ) {
                    std::cerr << name() << ": failed on "; estream.write( message );
                    COMMA_THROW( comma::exception, "found 'end' but missing FIFO entry 'begin' log entry with that name" );
                }
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
    try
    {
        comma::command_line_options options( ac, av, usage );

        if( options.exists( "--sum" ) )
        {
            
            typedef boost::property_tree::ptree::path_type ptree_path_type;
            boost::property_tree::ptree ptree; 
            typedef boost::unordered_set< std::string > leaf_keys_t;
            boost::unordered_set< std::string > leaf_keys;
            merge_elapsed( ptree, leaf_keys, options.exists( "--mean" ), options.exists( "--count" ) );
            
            if( options.exists( "--ratio" ) )  
            {
                
                // This is the optional path for base denominator
                std::vector< std::string > no_names = options.unnamed( "-h,--help,--elapsed,--sum,--mean,--count,--ratio,-P,--percentage", "" );
                std::string& denominator_path = no_names.front(); 
                double total_time = 0;
                if( !no_names.empty() && !denominator_path.empty() )
                {
                    if( denominator_path.size() < elapsed_end.size() || denominator_path.substr( denominator_path.size() - (elapsed_end.size()) ) != elapsed_end ) {
                        denominator_path += "/elapsed";
                    }
                    // This is the path for base denominator
                    const ptree_path_type key( denominator_path, '/' );
                    boost::optional< double > denominator = ptree.get_optional< double >( key );
                    
                    if( !denominator ) { COMMA_THROW( comma::exception, "failed to find path in input data: " + denominator_path ); }
                    total_time = *denominator;
                }
                else 
                {
                    // total time is total time of all childs of root node in tree/path
                    for( boost::property_tree::ptree::const_iterator i = ptree.begin(); i != ptree.end(); ++i )
                    {
                        /// search for elapsed because it is in the second ptree node under 'elapsed'
                        boost::optional< double > elapsed = i->second.get_optional< double >( ptree_path_type( "elapsed" ) );
                        if( elapsed ) { total_time += *elapsed; }
                    }
                }
                
                if( total_time == 0 ) { COMMA_THROW( comma::exception, "denominator time cannot be found" ); }
                
                const bool show_percentage = options.exists( "-P,--percentage" );
                // Now make ratio tree
                for( leaf_keys_t::const_iterator ikeystr=leaf_keys.cbegin(); ikeystr!=leaf_keys.cend(); ++ikeystr )
                {
                    const std::string& key_str = *ikeystr;
                    // Only if the key is '*/elapsed', do not make ratio key for other keys
                    if( key_str.size() < elapsed_end.size() || key_str.substr( key_str.size() - elapsed_end.size() ) != elapsed_end ) { continue; } 
                    
                    const ptree_path_type key( key_str, '/' );
                    std::string ratio_key_str =  key_str.substr(0, key_str.find_last_of( '/' ) );
                    const ptree_path_type ratio_key( ratio_key_str + '/' + "ratio", '/' );
                    
                    boost::optional< double > value = ptree.get_optional< double >( key );
                    if( !show_percentage ) {
                        ptree.put( ratio_key, (*value)/total_time );
                    }
                    else { 
                        ptree.put( ratio_key, boost::math::round( ( ((*value)*100.0)/total_time ) * 1000.0 ) / 1000.0 );
                    }
                }
            }
            
            comma::property_tree::to_path_value( std::cout, ptree, comma::property_tree::disabled, equal_sign, '\n' );
        }
        else if( options.exists( "--elapsed" ) )
        {
            boost::function< void( const impl_::log&, const impl_::log&, const std::string&) > outputting( &output_elapsed );
            boost::function< const impl_::log*() > extractor( &get_log );
            if( options.exists( "--from-path-value,--from-pv" ) )
            {
                extractor = &get_log_path_value;
            }
            impl_::process_begin_end< impl_::log >( extractor , outputting );
            
            return 0;
        }
        else
        {
            boost::function< const impl_::log*() > extractor( &get_log );
            boost::function< void( const impl_::log&, const impl_::log&, const std::string&) > outputting( &output );
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
