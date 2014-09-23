// g++ -o xml-map-split xml-map-split.cpp -lstdc++

#include <cassert>

#include <iostream>
#include <sstream>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <limits>
#include <vector>
#include <set>
#include <list>

#include <cstring>

#include "./stream_util.h"

using comma::io::punct;

#define CMDNAME "xml-map-split"

static std::set<std::string> exact_set;
static std::list<std::string> grep_list;
static unsigned block_start = 0;
static unsigned block_end = std::numeric_limits<unsigned>::max(); 

// ~~~~~~~~~~~~~~~~~~
// HELPERS
// ~~~~~~~~~~~~~~~~~~
// An istream manipulator to fill a string until the given punctuation
// will take in whitespace
// but leaves the whitespace in the stream.
struct before
{
    before(std::string & str, char const c) : _str(str), _c(c) {;}
    std::string & _str;
    char const _c;
};

std::istream &
operator >>(std::istream & is, before const & p)
{
    p._str.clear();
    for (;;)
    {
        char const c = is.get();
        if (! is)
            return is;

        if (p._c == c)
        {
            is.unget();
            return is;
        }
        p._str.push_back(c);
    }
}

bool
match(std::string const & v)
{
    // std::cerr << "Find " << v << std::endl;
    if (exact_set.end() != exact_set.find(v))
        return true;

    std::list<std::string>::const_iterator const end = grep_list.end();
    std::list<std::string>::const_iterator itr = grep_list.begin();
    for (; itr != end; ++itr)
        if (v.size() > itr->size()) // can't do a partial match if smaller
        {
            size_t pos = v.size() - itr->size();
            // std::cerr << "Compare to " << v.c_str() + pos << std::endl;
            if (0 == std::strcmp(v.c_str() + pos, itr->c_str()))
                return true;
        }

    return false;
}

// ~~~~~~~~~~~~~~~~~~
// USER INTERFACE
// ~~~~~~~~~~~~~~~~~~
static void
usage(bool const verbose)
{
    std::cerr <<   "USAGE:   " CMDNAME " <inputname> <mapname> <range> <xpath>+"
              << "\nWHERE:   <range> is 'all' or M-N"
              << "\nRETURNS: 0 - on success"
                 "\n         1 - on data error; like invalid xml"
              << std::endl;
}

// ~~~~~~~~~~~~~~~~~~
// MAIN
// ~~~~~~~~~~~~~~~~~~
static bool 
output_block(std::istream & infile, std::vector<char> & buffy, long long const start, long long stop)
{
    if (stop < start)
    {
        std::cerr << CMDNAME ": Error: Parsing: Start Offset is Less than Stop Offset" << std::endl;
        return false;
    }

    long long const len = stop - start;

    try {
        buffy.resize(len, 0);
    } catch(std::bad_alloc const & e) {
        std::cerr << CMDNAME ": Error: Unable to allocate buffer of " << len << std::endl;
        return false;
    }
    
    if (! infile.seekg(start))
    {
        std::cerr << CMDNAME ": Error: Unable to seek input file " << start << std::endl;
        return false;
    }
    if (! infile.read(&buffy[0], len))
    {
        std::cerr << CMDNAME ": Error: Unable to read input file." << std::endl;
        return false;
    }
    std::cout.write(&buffy[0], len);
    std::cout << std::endl;
    
    return true;
}

static bool
parse(std::istream & infile, std::istream & mapfile)
{
    std::vector<char> buffy;
    std::string path;
    path.reserve(1024);
    
    unsigned block_curr = 0;
    
    while (mapfile)
    {
        long long start = 0, stop = 0;
        path.clear();

        mapfile >> before(path, ',');
        if (! mapfile)
        {
            std::cerr << CMDNAME ": Error: Parsing: Failed to get an XPath then a comma." << std::endl;
            return false;
        }
        if (! match(path))
        {
            comma::io::ignore_until_endl(mapfile);
        }
        else
        {
            do
            {
                mapfile >> punct(',') >> start >> punct('-') >> stop;
                if (! mapfile)
                {
                    std::cerr << CMDNAME ": Error: Parsing: Failed to get a block location." << std::endl;
                    return false;
                }
                //DB std::cerr << path << '|' << start << '|' << stop << std::endl;

                ++block_curr;
                if (block_curr > block_end)
                    return true;

                //DB std::cerr << '@' << path << '|' << start << '|' << stop << std::endl;

                if (block_curr >= block_start)
                    if (! output_block(infile, buffy, start, stop))
                        return false;
            } while (',' == mapfile.peek());

            mapfile >> comma::io::endl;
            if (! mapfile)
            {
                std::cerr << CMDNAME ": Error: Parsing: Failed to get a newline." << std::endl;
                return false;
            }
        }
    }
    return true;
}

static int
run(char const * const infname, char const * const mapfname)
{
    std::ifstream infile;
    infile.open(infname, std::ios::in);
    if (! infile.good())
    {
        std::cerr << CMDNAME ": Error: Unable to open input file '" << infname << '\'' << std::endl;
        return 1;
    }
    
    std::ifstream mapfile;
    mapfile.open(mapfname, std::ios::in);
    if (! mapfile.good())
    {
        std::cerr << CMDNAME << ": Error: Unable to open map file '" << mapfname << '\'' << std::endl;
        return 1;
    }
    
    bool const ok = parse(infile, mapfile);

    return ok ? 0 : 1;
}

int
main(int argc, char ** argv)
{
    try
    {
        if (argc < 5)
        {
            usage(argv[0]);
            return 1;
        }
        
        if (0 != std::strcmp("all", argv[3]))
        {
            std::istringstream iss(argv[3]);
            unsigned s = 0, e = std::numeric_limits<unsigned>::max();
            iss >> s >> punct('-') >> e;
            block_start = std::min(s, e);
            block_end = std::max(s, e);
            //DB std::cerr << "Blocks (" << argv[3] << ") " << block_start << "-" << block_end << std::endl;
        }
        
        for (unsigned i = 4; i < unsigned(argc); ++i)
        {
            std::string const str(argv[i]);
            if (1 == str.length())
                exact_set.insert(str);
            else if ('/' != str[0])
                grep_list.push_back(str);
            else if ('/' != str[1])
                exact_set.insert(str);
            else 
                grep_list.push_back(str.substr(1)); // keep a preceeding slash because of namespaces
        }

        if (false)
        {
            std::ostream_iterator<std::string> out_itr(std::cerr, "\n");
            std::copy(exact_set.begin(), exact_set.end(), out_itr);
            std::copy(grep_list.begin(), grep_list.end(), out_itr);
        }
    
        return run(argv[1], argv[2]);
    }
    catch (std::exception const & ex)
    {
        std::cerr << CMDNAME ": Error: " << ex.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << CMDNAME ": Error: Unknown Exception." << std::endl;
    }
    return 1;
}

