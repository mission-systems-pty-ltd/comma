#include <cassert>

#include <iostream>
#include <limits>
#include <list>

#include <expat.h>

#include <comma/application/command_line_options.h>
#include <comma/xpath/xpath.h>

#define CMDNAME "xml-split"

static std::string options_file;

static unsigned const BUFFY_SIZE = 1 * 1024 * 1024;

static unsigned block_end = std::numeric_limits<unsigned>::max(); 

static unsigned element_count = 0;
static unsigned element_found_count = 0;
static unsigned element_depth = 0;
static unsigned element_depth_max = 0;

static XML_Parser parser = NULL;

// can't use list or map on xpath because of the < overloading
static std::list<comma::xpath> exact_list;
static std::list<comma::xpath> grep_list;

// ~~~~~~~~~~~~~~~~~~
// UTILITIES
// ~~~~~~~~~~~~~~~~~~
namespace comma
{
    // This needs to be in the comma namespace
    std::ostream &
    operator <<(std::ostream & os, comma::xpath const & p)
    {
        return p.output(os);
    }
}

// ~~~~~~~~~~~~~~~~~~
// USER INTERFACE
// ~~~~~~~~~~~~~~~~~~
static void XMLCALL
usage(bool const verbose)
{
    std::cerr <<   "Splits the file up into chunks based on the size, also does grep for efficiency"
                 "\nUSAGE:   " CMDNAME " [--limit=Q]  [--source=XMLFILE]"
                 "\nOPTIONS: --limit=Q to output just the elements between 1 and Q"
                 "\n         --source=XMLFILE to open and parse that file."
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
}

static void
element_end(void * userdata, char const * element)
{
    assert(NULL != element);

    --element_depth;
}

// ~~~~~~~~~~~~~~~~~~
// MAIN
// ~~~~~~~~~~~~~~~~~~
static bool
parse_as_blocks(FILE * infile)
{
    for (;;)
    {
        void * const buffy = XML_GetBuffer(parser, BUFFY_SIZE);
        if (NULL == buffy)
        {
            std::cerr << CMDNAME ": Error: Could not allocate expat parser buffer. Abort!" << std::endl;
            return false;
        }

        size_t const bytes_read = std::fread(buffy, 1, BUFFY_SIZE, infile);
        if (0 != bytes_read)
        {
            if (XML_STATUS_OK != XML_ParseBuffer(parser, bytes_read, bytes_read < BUFFY_SIZE))
            {
                std::cerr << CMDNAME ": " << XML_ErrorString(XML_GetErrorCode(parser)) << std::endl;
                std::cerr << CMDNAME ": Error: Parsing Buffer. Abort!" << std::endl;
                return false;
            }
        }

        if (std::ferror(infile))
        {
            std::cerr << CMDNAME ": Error: Could not read. Abort!" << std::endl;
            return false;
        }

        if (std::feof(infile))
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

    XML_SetElementHandler(parser, element_start, element_end);

    bool ok = false;
    if (options_file.empty())
    {
        ok = parse_as_blocks(stdin);
    }
    else
    {
        FILE * infile = fopen(options_file.c_str(), "rb");
        if (NULL != infile)
        {
            ok = parse_as_blocks(infile);
            fclose(infile);
        }
        else
        {
            std::cerr << CMDNAME ": Error: Could not open input file '" << options_file.c_str() << "'. Abort!" << std::endl;
        }
    }
    
    XML_ParserFree(parser);
    
    std::cerr << CMDNAME ": Number of Elements " << element_count
              << ", Number of Found Elements " << element_found_count
              << ", Maximum Depth " << element_depth_max << std::endl;
                  
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

        if (options.exists("--limit"))
        {
            unsigned e = options.value<unsigned>("--limit");
            block_end = std::max(1u, e);
        }
        if (options.exists("--source"))
        {
            options_file = options.value<std::string>("--source");
        }

        for (unsigned i = 1; i < unsigned(argc); ++i)
        {   
            //std::string const str(argv[i]);
            if (0 == argv[i][1]) // length of 1
                exact_list.push_back(comma::xpath(argv[i]));
            else if ('/' != argv[i][0])
                grep_list.push_back(comma::xpath(argv[i]));
            else if ('/' != argv[i][1])
                exact_list.push_back(comma::xpath(argv[i]));
            else 
                grep_list.push_back(comma::xpath(argv[i] + 2));
        }

        if (false)
        {
            std::ostream_iterator<comma::xpath const> out_itr(std::cerr, "\n");
            std::copy(exact_list.begin(), exact_list.end(), out_itr);
            std::copy(grep_list.begin(), grep_list.end(), out_itr);
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

