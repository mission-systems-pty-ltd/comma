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

static std::set<std::string> exact_set;
static std::list<std::string> grep_list;
static unsigned block_start = 0;
static unsigned block_end = std::numeric_limits<unsigned>::max(); 

// ~~~~~~~~~~~~~~~~~~
// HELPERS
// ~~~~~~~~~~~~~~~~~~
// An istream manipulator to remove the expected  newline 
// Somewhat complicated by that whole DOS v UNIX v MAC bulldust
struct newline_t
{
    newline_t() {;}
} newline;

std::istream &
operator >>(std::istream & is, newline_t const & p)
{
    char const c = is.get();
    if (! is || '\n' == c) return is;
    if ('\r' != c )
    {
        is.setstate(std::ios::failbit);
        return is;
    }
    int next = is.peek();
    if (EOF == next) return is;
    if ('\n' == next) next = is.get();
    return is;
}

// An istream manipulator to read a punctuation char or fail.
struct punct
{
    punct(char const c) : _c(c) {;}
    char const _c;
};

std::istream &
operator >>(std::istream & is, punct const & p)
{
    char const c = is.get();
    if (c != p._c) is.setstate(std::ios::failbit);
    return is;
}

// An istream manipulator to fill a string until the given punctuation
// will take in whitespace
struct until
{
    until(std::string & str, char const c) : _str(str), _c(c) {;}
    std::string & _str;
    char const _c;
};

std::istream &
operator >>(std::istream & is, until const & p)
{
    p._str.clear();
    for (;;)
    {
        char const c = is.get();
        if (! is || p._c == c) return is;
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
usage(char const * const argv0)
{
    assert(NULL != argv0);

    std::cout <<   "USAGE:   " << argv0 << " <inputname> <mapname> <range> <xpath>+"
              << "\nWHERE:   <range> is 'all' or M-N"
              << "\nRETURNS: 0 - on success"
                 "\n         1 - on data error; like invalid xml"
              << std::endl;
}

// ~~~~~~~~~~~~~~~~~~
// MAIN
// ~~~~~~~~~~~~~~~~~~
static int
parse(char const * const argv0, std::istream & infile, std::istream & mapfile)
{
    assert(NULL != argv0);

    std::vector<char> buffy;
    std::string path;
    path.reserve(1024);
    
    unsigned block_curr = 0;
    
    while (mapfile)
    {
        long long start = 0, stop = 0;
        path.clear();

        mapfile >> until(path, ',') >> start >> punct('-') >> stop >> newline;
        
        if (match(path))
        {
            // std::cerr << '@' << path << '|' << start << '|' << stop << std::endl;        

            ++block_curr;
            
            if (block_curr < block_start) continue;
            if (block_curr > block_end) return 0;

            long long len = stop - start;

            try {
                buffy.resize(len, 0);
            } catch(std::bad_alloc const & e) {
                std::cerr << argv0 << ": Error: Unable to allocate buffer of " << len << std::endl;
                return 1;
            }
            
            if (! infile.seekg(start))
            {
                std::cerr << argv0 << ": Error: Unable to seek input file " << start << std::endl;
                return 1;
            }
            if (! infile.read(&buffy[0], len))
            {
                std::cerr << argv0 << ": Error: Unable to read input file." << std::endl;
                return 1;
            }
            std::cout.write(&buffy[0], len);
            std::cout << std::endl;
        };
    }
}

int
main(int argc, char ** argv)
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
        // std::cerr << "Blocks (" << argv[3] << ") " << block_start << "-" << block_end << std::endl;
    }
    
    for (unsigned i = 4; i < argc; ++i)
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
    
    std::ifstream infile;
    infile.open(argv[1], std::ios::in);
    if (! infile.good())
    {
        std::cerr << argv[0] << ": Error: Unable to open input file '" << argv[1] << '\'' << std::endl;
        return 1;
    }
    
    std::ifstream mapfile;
    mapfile.open(argv[2], std::ios::in);
    if (! mapfile.good())
    {
        std::cerr << argv[0] << ": Error: Unable to open map file '" << argv[2] << '\'' << std::endl;
        return 1;
    }
    
    int code = parse(argv[0], infile, mapfile);

    return code;
}

