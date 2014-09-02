// g++ -o xml-xpath-grep xml-xpath-grep.cpp -lstdc++ -lexpat

#include <cassert>

#include <iostream>
#include <limits>
#include <vector>
#include <map>

#include <cstdio>
#include <cctype>

#include <expat.h>

#include <comma/application/command_line_options.h>

#define CMDNAME "xml-grep"

bool const TRIMWS = false;

static unsigned const BUFFY_SIZE = 4 * 1024 * 1024;

static unsigned block_start = 0;
static unsigned block_end = std::numeric_limits<unsigned>::max(); 
static unsigned block_curr = 0;

static unsigned element_count = 0;
static unsigned element_found_count = 0;
static unsigned element_depth = 0;
static unsigned element_depth_max = 0;

static XML_Parser parser = NULL;

typedef std::pair<std::string, bool /* relative */> grep_entry_t;
typedef std::vector<grep_entry_t> grep_list_t;

static grep_list_t grep_list;

static std::vector<std::string> element_list;
static unsigned element_found = 0;

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
            {
                ++element_found_count;
                return true;
            }
        }
        else
        {
            if (element_path == itr->first)
            {
                ++element_found_count;
                return true;
            }
        }

    return false;
}

// ~~~~~~~~~~~~~~~~~~
// USER INTERFACE
// ~~~~~~~~~~~~~~~~~~
static void
usage(bool const verbose)
{
    assert(NULL != argv0);

    std::cerr <<   "USAGE:   " CMDNAME " [--range=M-N] <path>"
              << "\nOPTIONS: --range=M-N to output just the blocks between M and N"
                 "\n         <path> is either absolute and fully qualified e.g. /n:a/n:b/n:c"
                 "\n                or it is fully qualified and realtive without subordinates e.g. n:c"
                 "\nRETURNS: 0 - on success"
                 "\n         1 - on data error; like invalid xml"
              << std::endl;
}

// ~~~~~~~~~~~~~~~~~~
// SAX HANDLERS
// ~~~~~~~~~~~~~~~~~~
static void XMLCALL
default_handler(void * userdata, XML_Char const * str, int length)
{
    assert(NULL != str);
    assert(length > 0);

    if (block_curr < block_start || block_curr > block_end)
        return;

    if (element_found > 0)
        std::cout.write(str, length);

    if (false)
    {
        bool was_ws = false;
        for (unsigned i = 0; i < unsigned(length); ++i)
            if (! std::isspace(str[i]))
                std::cout << str[i];
            else
            {
                if (! was_ws) std::cout << ' ';
                was_ws = true;
            }
    }
}

static void XMLCALL
comment(void * userdata, XML_Char const * str)
{
    // DO NOTHING
}

static void XMLCALL
element_start(void * userdata, XML_Char const * element, XML_Char const ** attributes)
{
    assert(NULL != element);
    assert(NULL != attributes);

    ++element_count;
    element_depth_max = std::max(element_depth_max, element_depth);
    ++element_depth;
    
    std::string element_path;
    element_path.reserve(4000);
    if (! element_list.empty())
        element_path = element_list.back();
    element_path.append("/");
    element_path.append(element);
    element_list.push_back(element_path);
    
    if (grep(element, element_path))
    {
        ++element_found;
        ++block_curr;
    }
    
    if (block_curr < block_start || block_curr > block_end)
        return;

    if (element_found > 0)
    {
        std::cout << '<' << element << ' ';
        if (NULL != attributes)
            for (unsigned i = 0; NULL != attributes[i]; i += 2)
            {
                std::cout << attributes[i] << "='";
                for (XML_Char const * p = attributes[i+1]; *p != 0; ++p)
                    if ('\'' == *p)
                        std::cout << "&apos;";
                    else
                        std::cout << *p;
                std::cout << "' ";
            }
        std::cout << '>';
    }
}

static void
element_end(void * userdata, XML_Char const * element)
{
    assert(NULL != element);

    std::string const & element_path = element_list.back();
    bool const was_found = element_found > 0;

    if (was_found)
        if (block_curr >= block_start && block_curr <= block_end)
            std::cout << "</" << element << '>';
    
    if (grep(element, element_path))
        --element_found;
        
    if (was_found && 0 == element_found)
        if (block_curr >= block_start && block_curr <= block_end)
            std::cout << std::endl;

    element_list.pop_back();
    --element_depth;

    if (block_curr > block_end)
        XML_StopParser(parser, false);
}

// ~~~~~~~~~~~~~~~~~~
// MAIN
// ~~~~~~~~~~~~~~~~~~
static bool
parse()
{
    XML_SetElementHandler(parser, element_start, element_end);
    XML_SetDefaultHandler(parser, default_handler);
    XML_SetCommentHandler(parser, comment);

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
                if (block_curr <= block_end)
                    std::cerr << CMDNAME ": Error: Parsing Buffer. Abort!" << std::endl;
                std::cerr << std::endl;
                return false;
            }
        }

        if (std::ferror(stdin))
        {
            std::cerr << CMDNAME ": Error: Could not read stdin. Abort!" << std::endl;
            return false;
        }

        if (std::feof(stdin))
        {
            return true;
        }
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

    if (block_start > element_count)
    {
        std::cerr << CMDNAME ": Error: Block start out of range (" << element_count << "). Abort!" << std::endl;
        return 1;
    }
              
    if (ok)
        std::cerr << CMDNAME ": Number of Elements " << element_count
                  << ", Number of Found Elements  " << element_found_count
                  << ", Maximum Depth " << element_depth_max << std::endl;
                  
    return ok ? 0 : 1;
}

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
            iss >> s >> punct('-') >> e;
            block_start = std::min(s, e);
            block_end = std::max(s, e);

            // std::cerr << "Blocks " << block_start << '-' << block_end << std::endl;
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

