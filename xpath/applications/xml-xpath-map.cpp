// g++ -std=c++0x -o xml-xpath-map xml-xpath-map.cpp -lstdc++ -lexpat

#include <iostream>
#include <limits>
#include <vector>
#include <map>

#include <cstdio>

#include <expat.h>

static unsigned const BUFFY_SIZE = 64 * 1024 * 1024;

static unsigned element_count = 0;
static unsigned element_depth = 0;
static unsigned element_depth_max = 0;

static XML_Parser parser = nullptr;

static std::vector<std::string> element_path_list;
// static std::string element_path;
static std::map<std::string,std::vector<std::pair<long long, long long>>> element_map;

// ~~~~~~~~~~~~~~~~~~
// USER INTERFACE
// ~~~~~~~~~~~~~~~~~~
static void XMLCALL
usage(char const * const argv0)
{
    std::cout <<   "USAGE: " << argv0 
              << "\nRETURNS: 0 - on success"
                 "\n         1 - on data error; like invalid xml"
                 "\n         2 - on simple error; like incorrect argument"
                 "\n         3 - on internal error; like memory / library fault"
              << std::endl;
}

// ~~~~~~~~~~~~~~~~~~
// SAX HANDLERS
// ~~~~~~~~~~~~~~~~~~
static void XMLCALL
element_start(void * userdata, char const * element, char const ** attributes)
{
    ++element_count;
    element_depth_max = std::max(element_depth_max, element_depth);
    ++element_depth;
    
    std::string element_path;
    element_path.reserve(4000);
    if (! element_path_list.empty())
        element_path = element_path_list.back();
    element_path.append("/");
    element_path.append(element);
    
    element_path_list.push_back(std::string(element_path));
    
    long long const at = XML_GetCurrentByteIndex(parser);
    std::pair<long long, long long> loc(at,0);
    element_map[element_path].push_back(loc);

    // std::cout << "+ " << element_path << ' ' << at << std::endl;
}

static void
element_end(void * userdata, char const * element)
{
    long long const was = element_map[element_path_list.back()].back().first;
    long long const at = XML_GetCurrentByteIndex(parser);
    std::cout << element_path_list.back() << ' ' << was << '-' << at << std::endl;

    element_map[element_path_list.back()].back().second = at;

    --element_depth;
    element_path_list.pop_back();

}

// ~~~~~~~~~~~~~~~~~~
// MAIN
// ~~~~~~~~~~~~~~~~~~
int parse(char const * const argv0, XML_Parser const parser)
{
    XML_SetElementHandler(parser, element_start, element_end);

    for (;;)
    {
        void * const buffy = XML_GetBuffer(parser, BUFFY_SIZE);
        if (nullptr == buffy)
        {
            std::cout << argv0 << "Error: Could not allocate expat parser buffer. Abort!" << std::endl;
            return 3;
        }

        size_t const bytes_read = std::fread(buffy, 1, BUFFY_SIZE, stdin);
        if (0 != bytes_read)
        {
            if (XML_STATUS_OK != XML_ParseBuffer(parser, bytes_read, bytes_read < BUFFY_SIZE))
            {
                std::cout << argv0 << "Error: Parsing Buffer. Abort!" << std::endl;
                return 2;
            }
        }

        if (std::ferror(stdin))
        {
            std::cout << argv0 << "Error: Could not read stdin. Abort!" << std::endl;
            return 2;
        }

        if (std::feof(stdin))
        {
            return 0;
        }
    }
}

int main(int argc, char ** argv)
{
    if (1 != argc)
    {
        usage(argv[0]);
        return 2;
    }

    parser = XML_ParserCreate(NULL);
    if (nullptr == parser)
    {
        std::cout << argv[0] << "Error: Could not create expat parser. Abort!" << std::endl;
        return 3;
    }

    int const code = parse(argv[0], parser);
    
    XML_ParserFree(parser);
    
    std::cerr << "Number of Elements " << element_count << ", Maximum Depth " << element_depth_max << std::endl;

    if (false)
    {
        for (auto & kv : element_map)
        {
            std::cout << kv.first;
            for (auto & se : kv.second)
            {
                std::cout << ',' << se.first << ',' << se.second;
            }
            std::cout << '\n';
        }
    }
        
    return code;
}

