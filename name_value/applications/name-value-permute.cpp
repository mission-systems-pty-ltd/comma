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

#include <iostream>
#include <fstream>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "../../base/exception.h"
#include "../../application/command_line_options.h"
#include "../../string/split.h"

static void usage( bool verbose = false )
{
    std::cerr << std::endl;
    std::cerr << "take path-value input including lists of values, and output each permutation of those lists" << std::endl;
    std::cerr << "path-value pairs that do not include permutations are passed through unchanged" << std::endl;
    std::cerr << std::endl;
    std::cerr << "output:" << std::endl;
    std::cerr << "    if --stdout is specified, then permutations are indicated by permutations[<count>]/<path>=<value>" << std::endl;
    std::cerr << "    otherwise, each permutation will be written to a separate file: <count>.path-value" << std::endl;
    std::cerr << std::endl;
    std::cerr << "permutations:" << std::endl;
    std::cerr << "    permutations can be a combination of lists and ranges" << std::endl;
    std::cerr << "    for a list, values are separated with ',' (or use --delim option to set a delimiter other than , )" << std::endl;
    std::cerr << "    for a range, specify with <start>:<stop>[:<step>] (use --range-delim to set a delimiter other than : )" << std::endl;
    std::cerr << "    valid types for ranges are int, double and time (time step is in seconds)" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options:" << std::endl;
    std::cerr << "    --help,-h; output help and exit" << std::endl;
    std::cerr << "    --delimiter,-d=<delimiter>;default=','; delimiter for lists" << std::endl;
    std::cerr << "    --range-delimiter=<delimiter>;default=':'; delimiter for ranges" << std::endl;
    std::cerr << "    --prefix=<prefix>;default='permutations'; prefix to add to paths or filenames" << std::endl;
    std::cerr << "    --stdout; output to stdout as permutation array; default: output to files" << std::endl;
    std::cerr << "examples:" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    echo 'list=a,b,c' | name-value-permute" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    echo 'range=1-3' | name-value-permute --range-delimiter='-'" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    echo 'number=684:690:2" << std::endl;
    std::cerr << "          string=\"a,\";b;c" << std::endl;
    std::cerr << "          unchanged=unchanged' | name-value-permute --stdout --delimiter=';'" << std::endl;
    std::cerr << std::endl;
    std::cerr << std::endl;
    exit( 0 );
}

struct map_t
{
    std::string path;
    std::vector<std::string> values;
};

void update_index(const std::vector<map_t>& permutations, std::vector<std::size_t>& index)
{
    for (std::size_t i = permutations.size(); i-- > 0;)
    {
        index[i]++;
        if (index[i] == permutations[i].values.size()) { index[i] = 0; continue; }
        break;
    }
}

void output_to_stdout(const std::vector<map_t>& permutations, const comma::command_line_options& options )
{
    std::string prefix = options.value<std::string>("--prefix", "permutations");
    std::size_t count = 0;
    std::size_t total_count = 1;
    for ( std::size_t p = 0; p < permutations.size(); ++p ) { total_count = total_count * permutations[p].values.size(); }
    std::vector <std::size_t> index(permutations.size());
    while (count < total_count)
    {
        for (std::size_t p = 0; p < permutations.size(); ++p)
        {
            std::cout << prefix << "[" << count << "]/" << permutations[p].path << "=" << permutations[p].values[index[p]] << std::endl;
        }
        update_index(permutations, index);
        count++;
    }
}

void output_to_files(const std::vector<map_t>& permutations, const comma::command_line_options& options )
{
    std::size_t count = 0;
    std::size_t total_count = 1;
    for ( std::size_t p = 0; p < permutations.size(); ++p ) { total_count = total_count * permutations[p].values.size(); }
    
    std::string prefix = options.value<std::string>("--prefix", "");
    
    std::string dot = prefix.empty() ? "" : ".";
    
    // vector containing the current position in each input 
    std::vector <std::size_t> index(permutations.size());
    while (count < total_count)
    {
        std::string filename = prefix + dot + boost::lexical_cast<std::string>(count) + ".path-value"; 
        std::ofstream ostream(filename.c_str());
        for (std::size_t p = 0; p < permutations.size(); ++p)
        {
            ostream << permutations[p].path << "=" << permutations[p].values[index[p]] << std::endl;
        }
        ostream.close();
        update_index(permutations, index);
        count++;
    }
}

int main( int ac, char** av )
{
    std::string line;
    try
    {
        comma::command_line_options options( ac, av, usage );
        char delimiter = options.value< char >( "--delimiter,--delim,-d", ',' );
        char range_delimiter = options.value< char >( "--range-delimiter,--range-delim,-r", ':' );
        
        // read each input line.
        std::string line;
        
        std::vector< map_t > permutations;
        
        while (std::getline(std::cin, line))
        {
            if ( line.empty() ) { continue; }
            // ignored comments
            if ( line[0] == '#' ) { continue; }
            // get path and value. ( split at first = )
            std::string::size_type p = line.find_first_of( '=' );
            if( p == std::string::npos ) { COMMA_THROW( comma::exception, "expected '" << delimiter << "'-separated xpath=value pairs; got \"" << line << "\"" ); }
            const std::string& path = line.substr( 0, p );
            const std::string& value = line.substr( p + 1, std::string::npos );
            // use split_escaped to split by delimiter
            std::vector<std::string> values = comma::split_escaped(value, delimiter);
            
            map_t map;
            map.path = path;
            
            for (std::size_t i = 0; i < values.size(); ++i )
            {
                // multi values.
                //    for each value:
                //       split_escaped to check if it is a range
                
                std::vector<std::string> range = comma::split_escaped(values[i], range_delimiter);
                if ( range.size() == 1 )
                {
                    // if is not a range, add to std::vector<std::string> of values associated with the path.
                    map.values.push_back(values[i]);
                }
                else
                {
                    if (range.size() < 2 || range.size() > 3) 
                    { COMMA_THROW(comma::exception, "expected range is \"start" << range_delimiter << "stop" << range_delimiter << "step\", got \"" << values[i] << "\"" << std::endl ); }
                    
                    
                    try {
                        double start = boost::lexical_cast<double>(range[0]);
                        double stop = boost::lexical_cast<double>(range[1]);
                    
                        if ( !boost::math::isfinite(start) || !boost::math::isfinite(stop) ) 
                        { COMMA_THROW(comma::exception, "cannot expand range " << values[i] << std::endl ); }
                    
                        int sign = start < stop ? 1 : -1 ;
                        double step;
                        if ( range.size() == 3 ) { step = boost::lexical_cast<double>(range[2]); }
                        else { step = sign; }
                    
                        if ( step == 0 || ( sign * step < 0 ) ) { COMMA_THROW(comma::exception, "cannot expand range " << values[i] << std::endl) }
                    
                        // if it is a range: parse begin, end, and step (default 1)
                        // todo: support int, double, time??
                    
                        // throw on infinity, step = 0, sign mismatch
                        // for (i = start; i <= stop; i += step ) // (need to check for -ve step)
                        // push the value onto std::vector<std::string> of values associated with path
                        double i = start;
                        do
                        {
                            std::string s = boost::lexical_cast<std::string>(i);
                            map.values.push_back(s);
                            i += step;
                        } while ( ( sign > 0 && i < stop ) || ( sign < 0 && i > stop ) );
                    }
                    catch ( boost::bad_lexical_cast & e )
                    {
                        boost::posix_time::ptime start = boost::posix_time::from_iso_string(range[0]);
                        boost::posix_time::ptime stop = boost::posix_time::from_iso_string(range[1]);
                    
                        int sign = start < stop ? 1 : -1 ;
                        double step;
                        if ( range.size() == 3 ) { step = boost::lexical_cast<double>(range[2]); }
                        else { step = sign; }
                    
                        if ( step == 0 || ( sign * step < 0 ) ) { COMMA_THROW(comma::exception, "cannot expand range " << values[i] << std::endl) }
                    
                        // if it is a range: parse begin, end, and step (default 1)
                        // todo: support int, double, time??
                    
                        // throw on infinity, step = 0, sign mismatch
                        // for (i = start; i <= stop; i += step ) // (need to check for -ve step)
                        // push the value onto std::vector<std::string> of values associated with path
                        boost::posix_time::ptime i = start;
                        do
                        {
                            std::string s = boost::posix_time::to_iso_string(i);
                            map.values.push_back(s);
                            i += boost::posix_time::seconds(static_cast<unsigned int>(step));
                        } while ( ( sign > 0 && i < stop ) || ( sign < 0 && i > stop ) );
                    }
                }
            }
            permutations.push_back(map);
        }
        
        if (options.exists("--stdout"))
        {
            output_to_stdout(permutations, options);  
        }
        else
        {
            output_to_files(permutations, options);
        }
        
        return 0;
    }
    catch( std::exception& ex )
    { 
        std::cerr << "name-value-permute: " << ex.what() << std::endl;
    }
    catch( ... )
    { 
        std::cerr << "name-value-permute: unknown exception" << std::endl;
    }
    return 1;
}
