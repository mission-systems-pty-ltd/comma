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
#include <list>
#include <map>

#include <cstdio>
#include <cstring>

#include <expat.h>

#include <comma/application/command_line_options.h>
#include <comma/io/stream-util.h>
#include <comma/xpath/xpath.h>
#include <comma/xpath/applications/expat-util.h>

// ~~~~~~~~~~~~~~~~~~
// Locals
// ~~~~~~~~~~~~~~~~~~
#define CMDNAME "xml-map"

static std::string options_file;
static bool options_compact = false;
static unsigned options_depth_max = std::numeric_limits<unsigned>::max();

static unsigned const BUFFY_SIZE = 1 * 1024 * 1024;

static std::list<comma::xpath> element_path_list;

typedef std::pair<long long, long long> element_location_t;
typedef std::vector<element_location_t> element_location_list_t;
typedef std::map<comma::xpath, element_location_list_t, comma::xpath::less_t> element_location_map_t;
static element_location_map_t element_location_map;

// ~~~~~~~~~~~~~~~~~~
// UTILITIES
// ~~~~~~~~~~~~~~~~~~
inline std::ostream &
operator <<(std::ostream & os, comma::xpath const & p)
{
    return p.output(os);
}

// ~~~~~~~~~~~~~~~~~~
// USER INTERFACE
// ~~~~~~~~~~~~~~~~~~
static void XMLCALL
usage(bool const verbose)
{
    std::cerr <<   "Generates a byte map of each element in an xml file, for later use by xml-map-split"
                 "\nUSAGE:   " CMDNAME " [--compact] [--maxdepth=N] [--source=XMLFILE]"
                 "\nRETURNS: 0 - on success"
                 "\n         1 - on data error; like invalid xml"
              << std::endl;
}

// ~~~~~~~~~~~~~~~~~~
// APPLICATION
// ~~~~~~~~~~~~~~~~~~
class xml_map_application : public simple_expat_application
{
public:
    xml_map_application();

protected:
    virtual void
    do_element_start(char const * const element, char const * const * const attributes);

    virtual void
    do_element_end(char const * const element);
  
};

xml_map_application::xml_map_application()
: simple_expat_application(CMDNAME)
{
}

void
xml_map_application::do_element_start(char const * const element, char const * const * const attributes)
{
    if (element_depth <= options_depth_max)
    {
        ++element_found_count;
    
        // build the xpath
        comma::xpath element_path;
        if (! element_path_list.empty())
            element_path = element_path_list.back();
        element_path /= std::string(element);
        element_path_list.push_back(element_path);
        
        // get the start location
        long long const at = XML_GetCurrentByteIndex(parser);
        element_location_t loc(at, 0);
        // push the start location into the map
        element_location_map[element_path].push_back(loc);
    }
}

void
xml_map_application::do_element_end(char const * const element)
{
    if (element_depth <= options_depth_max)
    {    
        comma::xpath & element_path = element_path_list.back();
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
    }
}

// ~~~~~~~~~~~~~~~~~~
// MAIN
// ~~~~~~~~~~~~~~~~~~
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

        if (options.exists("--source"))
        {
            options_file = options.value<std::string>("--source");
        }
        options_compact = options.exists("--compact");
        options_depth_max = options.value<unsigned>("--maxdepth", std::numeric_limits<unsigned>::max());

        xml_map_application app;
        
        int const code = app.run(options_file);

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

        return code;
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

