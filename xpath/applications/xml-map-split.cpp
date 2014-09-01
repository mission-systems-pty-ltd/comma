// g++ -o xml-map-split xml-map-split.cpp -lstdc++

#include <cassert>

#include <iostream>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <vector>
#include <set>

std::set<std::string> grep_set;

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
    char c = 0; is >> c;
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

template<typename T>
bool
contains(std::set<T> const & m, T const & v)
{
    // std::cerr << "Find " << v << std::endl;
    return m.end() != m.find(v);
}

// ~~~~~~~~~~~~~~~~~~
// USER INTERFACE
// ~~~~~~~~~~~~~~~~~~
static void
usage(char const * const argv0)
{
    assert(NULL != argv0);

    std::cout <<   "USAGE: " << argv0 << " <inputname> <mapname> <xpath>+"
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
    
    while (mapfile)
    {
        long long start = 0, stop = 0;
        path.clear();

        mapfile >> until(path, ',') >> start >> punct('-') >> stop >> newline;
        
        if (contains(grep_set, path))
        {
            // std::cerr << '@' << path << '|' << start << '|' << stop << std::endl;        

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
    if (argc < 4)
    {
        usage(argv[0]);
        return 1;
    }
    
    for (unsigned i = 3; i < argc; ++i)
        grep_set.insert(std::string(argv[i]));

    if (false)        
    {
        std::ostream_iterator<std::string> out_itr(std::cerr, "\n");
        std::copy(grep_set.begin(), grep_set.end(), out_itr);
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

