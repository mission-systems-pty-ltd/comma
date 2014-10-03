// g++ -o xml-xpath-grep xml-xpath-grep.cpp -lstdc++ -lexpat

#include <cassert>

#include <limits>
#include <vector>
#include <list>
#include <map>

#include <cstdio>

#include <expat.h>

#include <comma/application/command_line_options.h>
#include "./expat_util.h"
#include "./stream_util.h"

#define CMDNAME "xml-grep"

static unsigned const BUFFY_SIZE = 1 * 1024 * 1024;

static unsigned block_start = 0;
static unsigned block_end = std::numeric_limits<unsigned>::max(); 
static unsigned block_curr = 0;

typedef std::pair<std::string, bool /* relative */> grep_entry_t;
typedef std::vector<grep_entry_t> grep_list_t;
static grep_list_t grep_list;
static unsigned element_found = 0;

// ~~~~~~~~~~~~~~~~~~
// UTILITIES
// ~~~~~~~~~~~~~~~~~~
bool
grep(XML_Char const * element, std::string const & element_path)
{
    // std::cerr << element << std::endl;

    grep_list_t::const_iterator const end = grep_list.end();
    grep_list_t::const_iterator itr = grep_list.begin();
    
    for (; itr != end; ++itr)
        if (itr->second) // relative
        {
            if (element == itr->first)
                return true;
        }
        else
        {
            if (element_path == itr->first)
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
    fputs("USAGE:   " CMDNAME " [--range=M-N] [--limit=Q] [--file=XMLFILE] PATH"
          "\nOPTIONS: --range=M-N to output just the elements between M and N"
          "\n         --limit=N to output just the elements between 1 and Q"
          "\n         --source=XMLFILE to open and parse that file."
          "\n         <path> is either absolute and fully qualified e.g. /n:a/n:b/n:c"
          "\n                or it is fully qualified and realtive without subordinates e.g. n:c"
          "\nRETURNS: 0 - on success"
          "\n         1 - on data error; like invalid xml"
          "\n",
          stderr);
}

// ~~~~~~~~~~~~~~~~~~
// APPLICATION
// ~~~~~~~~~~~~~~~~~~
class xml_grep_application : public simple_expat_application
{
public:
    xml_grep_application();

protected:
    virtual void 
    do_default(XML_Char const * const str, int const length);

    virtual void
    do_element_start(char const * const element, char const * const * const attributes);

    virtual void
    do_element_end(char const * const element);
  
};

xml_grep_application::xml_grep_application()
: simple_expat_application(CMDNAME)
{
}

void 
xml_grep_application::do_default(XML_Char const * const str, int const length)
{
    if (block_curr < block_start || block_curr > block_end)
        return;

    if (element_found > 0)
        fwrite(str, 1, length, stdout);
}

void
xml_grep_application::do_element_start(char const * const element, char const * const * const attributes)
{
    std::string const & element_path = current_xpath().to_string();
    
    if (grep(element, element_path))
    {
        ++element_found_count;
        ++element_found;
        ++block_curr;
    }
    
    if (block_curr < block_start || block_curr > block_end)
        return;

    if (element_found > 0)
    {
        fputc('<', stdout);
        fputs(element, stdout);
        fputc(' ', stdout);
        if (NULL != attributes)
            for (unsigned i = 0; NULL != attributes[i]; i += 2)
            {
                fputs(attributes[i], stdout);
                fputs("='", stdout);
                for (XML_Char const * p = attributes[i+1]; *p != 0; ++p)
                    if ('\'' == *p)
                        fputs("&apos;", stdout);
                    else
                        fputc(*p, stdout);
                fputs("' ", stdout);
            }
        fputc('>', stdout);
    }
}

void
xml_grep_application::do_element_end(char const * const element)
{
    std::string const & element_path = current_xpath().to_string();
    bool const was_found = element_found > 0;

    if (was_found)
        if (block_curr >= block_start && block_curr <= block_end)
        {
            fputs("</", stdout);
            fputs(element, stdout);
            fputc('>', stdout);
        }
    
    if (grep(element, element_path))
        --element_found;
        
    if (was_found && 0 == element_found)
        if (block_curr >= block_start && block_curr <= block_end)
            fputc('\n', stdout);

    if (block_curr > block_end)
        XML_StopParser(parser, false);
}

// ~~~~~~~~~~~~~~~~~~
// MAIN
// ~~~~~~~~~~~~~~~~~~
int
main(int argc, char ** argv)
{
    try
    {
        comma::command_line_options options( argc, argv );
        bool const verbose = options.exists( "--verbose,-v" );
        if (argc < 2 || options.exists("--help,-h"))
        {
            usage(verbose);
            return 1;
        }

        if (options.exists("--range"))
        {
            std::istringstream iss(options.value<std::string>("--range"));
            unsigned s = 0, e = std::numeric_limits<unsigned>::max();
            iss >> s >> comma::io::punct('-') >> e;
            block_start = std::min(s, e);
            block_end = std::max(s, e);
        }
        if (options.exists("--limit"))
        {
            unsigned e = options.value<unsigned>("--limit");
            block_end = std::max(1u, e);
        }

        std::string options_file;
        if (options.exists("--source"))
        {
            options_file = options.value<std::string>("--source");
        }

        for (unsigned i = 1; i < unsigned(argc); ++i)
        {   
            grep_entry_t curr(argv[i], false);
            
            // is realtive test
            if ("/" == curr.first || "//" == curr.first)
            {
                usage(argv[0]);
                return 1;
            }
            curr.second = curr.first[0] != '/';
        
            grep_list.push_back(curr);
        }
        
        xml_grep_application app;
        
        int const code = app.run(options_file);

        if (block_start > app.count_of_elements())
        {
            std::cerr << CMDNAME ": error: block start out of range (" << app.count_of_elements() << ")" << std::endl;
            return 1;
        }       
        
        return code;
    }
    catch (std::exception const & ex)
    {
        std::cerr << "xml-grep: " << ex.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "xml-grep: unknown exception" << std::endl;
    }
    return 1;
}

