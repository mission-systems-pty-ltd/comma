// g++ -o xml-xpath-map xml-xpath-map.cpp -lstdc++ -lexpat

// NOTE the original output format was thought to be used as arguments to cut
//         path1,s1,e1,s2,e2,s3,e3
//         path2,s1,e1,s2,e2,s3,e3
// However this exploded to massively long lines and so could not be used.
// Now it is one line per element

#include <cassert>

#include <iostream>
#include <limits>
#include <vector>
#include <map>

#include <cstdio>
#include <cstring>

#include <expat.h>

#include <comma/application/command_line_options.h>

#define CMDNAME "xml-map"

static bool options_compact=false;

static unsigned const BUFFY_SIZE = 4 * 1024 * 1024;

static unsigned element_count = 0;
static unsigned element_depth = 0;
static unsigned element_depth_max = 0;

static XML_Parser parser = NULL;

static std::vector<std::string> element_path_list;

typedef std::pair<long long, long long> element_location_t;
typedef std::vector<element_location_t> element_location_list_t;
typedef std::map<std::string, element_location_list_t> element_location_map_t;
static element_location_map_t element_location_map;

// ~~~~~~~~~~~~~~~~~~
// USER INTERFACE
// ~~~~~~~~~~~~~~~~~~
static void XMLCALL
usage(bool const verbose)
{
    std::cerr <<   "Generates a byte map of each element in an xml file, for later use by xml-map-split"
                 "\nUSAGE:   " CMDNAME " [--compact]"
                 "\nRETURNS: 0 - on success"
                 "\n         1 - on data error; like invalid xml"
              << std::endl;
}

// ~~~~~~~~~~~~~~~~~~
// SAX HANDLERS
// ~~~~~~~~~~~~~~~~~~
static void XMLCALL
element_start(void * userdata, char const * element, char const ** attributes)
{
    assert(NULL != element);
    assert(NULL != attributes);

    // track number of elements
    ++element_count;
    element_depth_max = std::max(element_depth_max, element_depth);
    ++element_depth;

    // build the xpath
    std::string element_path;
    element_path.reserve(4000);
    if (! element_path_list.empty())
        element_path = element_path_list.back();
    element_path.append("/");
    element_path.append(element);
    element_path_list.push_back(element_path);
    
    // get the start location
    long long const at = XML_GetCurrentByteIndex(parser);
    element_location_t loc(at, 0);
    // push the start location into the map
    element_location_map[element_path].push_back(loc);
}

static void
element_end(void * userdata, char const * element)
{
    assert(NULL != element);
    
    std::string const & element_path = element_path_list.back();
    element_location_t & entry = element_location_map[element_path].back();

    { // force the use of the entry to prevent errors
        long long txtlen = 3 + std::strlen(element);
        long long const at = XML_GetCurrentByteIndex(parser) + txtlen;

        entry.second = at;
    }
    
    if (! options_compact)
    {
        std::cout << element_path << ',' << entry.first << '-' << entry.second << std::endl;
    }
    // std::cerr << element_path << ',' << entry.first << '-' << entry.second << std::endl;

    element_path_list.pop_back();
    --element_depth;
}

// ~~~~~~~~~~~~~~~~~~
// MAIN
// ~~~~~~~~~~~~~~~~~~
static bool
parse()
{
    assert(NULL != argv0);

    XML_SetElementHandler(parser, element_start, element_end);

    for (;;)
    {
        void * const buffy = XML_GetBuffer(parser, BUFFY_SIZE);
        if (NULL == buffy)
        {
            std::cerr << CMDNAME ": Error: Could not allocate expat parser buffer. Abort!" << std::endl;
            return false;
        }

        size_t const bytes_read = std::fread(buffy, 1, BUFFY_SIZE, stdin);
        if (0 != bytes_read)
        {
            if (XML_STATUS_OK != XML_ParseBuffer(parser, bytes_read, bytes_read < BUFFY_SIZE))
            {
                std::cerr << CMDNAME ": Error: Parsing Buffer. Abort!" << std::endl;
                return false;
            }
        }

        if (std::ferror(stdin))
        {
            std::cerr << CMDNAME ": Error: Could not read stdin. Abort!" << std::endl;
            return false;
        }

        if (std::feof(stdin))
            return true;
    }
}

static int
run()
{
    parser = XML_ParserCreate(NULL);
    if (NULL == parser)
    {
        std::cerr << CMDNAME ": Error: Could not create expat parser. Abort!" << std::endl;
        return 1;
    }

    bool const ok = parse();
    
    XML_ParserFree(parser);
    
    if (options_compact)
    {
        element_location_map_t::const_iterator const end = element_location_map.end();
        element_location_map_t::const_iterator itr = element_location_map.begin();
        for (; itr != end; ++itr)
        {
            std::cout << itr->first;
            
            element_location_list_t::const_iterator const loc_end = itr->second.end();
            element_location_list_t::const_iterator loc_itr = itr->second.begin();
            for (; loc_itr != loc_end; ++loc_itr)
            {
                std::cout << ',' << loc_itr->first << '-' << loc_itr->second;
            }
            std::cout << '\n';
        }
    }

    std::cerr << CMDNAME ": Number of Elements " << element_count << ", Maximum Depth " << element_depth_max << std::endl;
                  
    return ok ? 0 : 1;
}

int main(int argc, char ** argv)
{
    try
    {
        comma::command_line_options options( argc, argv );
        bool const verbose = options.exists( "--verbose,-v" );
        if (options.exists("--help,-h"))
        {
            usage(verbose);
            return 1;
        }
        
        options_compact=options.exists("--compact");

        return run();
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

