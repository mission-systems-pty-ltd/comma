// g++ -o xml-xpath-grep xml-xpath-grep.cpp -lstdc++ -lexpat

#include <cassert>

#include <limits>
#include <vector>
#include <map>

#include <cstdio>
#include <cctype>

#include <expat.h>

#include <comma/application/command_line_options.h>

#define CMDNAME "xml-grep"

static unsigned const BUFFY_SIZE = 4 * 1024 * 1024;

static std::string options_file;

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

// ~~~~~~~~~~~~~~~~~~
// UTILITIES
// ~~~~~~~~~~~~~~~~~~
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
    fputs("USAGE:   " CMDNAME " [--range=M-N] [--limit=Q] [--file=XMLFILE] PATH"
          "\nOPTIONS: --range=M-N to output just the blocks between M and N"
          "\n         --limit=N to output just the blocks between 1 and Q-1"
          "\n         --source=XMLFILE to open and parse that file."
          "\n         <path> is either absolute and fully qualified e.g. /n:a/n:b/n:c"
          "\n                or it is fully qualified and realtive without subordinates e.g. n:c"
          "\nRETURNS: 0 - on success"
          "\n         1 - on data error; like invalid xml"
          "\n",
          stderr);
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
        fwrite(str, 1, length, stdout);
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

static void
element_end(void * userdata, XML_Char const * element)
{
    assert(NULL != element);

    std::string const & element_path = element_list.back();
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

    element_list.pop_back();
    --element_depth;

    if (block_curr > block_end)
        XML_StopParser(parser, false);
}

// ~~~~~~~~~~~~~~~~~~
// MAIN
// ~~~~~~~~~~~~~~~~~~
// this is useful for parsing a file.
static bool
parse_as_blocks(FILE * infile)
{
    for (;;)
    {
        void * const buffy = XML_GetBuffer(parser, BUFFY_SIZE);
        if (NULL == buffy)
        {
            fputs(CMDNAME ": Error: Could not allocate expat parser buffer. Abort!\n", stderr);
            return false;
        }

        size_t const bytes_read = std::fread(buffy, 1, BUFFY_SIZE, infile);
        if (0 != bytes_read)
        {
            if (XML_STATUS_OK != XML_ParseBuffer(parser, bytes_read, bytes_read < BUFFY_SIZE))
            {
                if (block_curr <= block_end)
                {
                    fprintf(stderr, CMDNAME ": %s\n", XML_ErrorString(XML_GetErrorCode(parser)));
                    fputs(CMDNAME ": Error: Parsing Buffer. Abort!\n", stderr);
                }
                fputc('\n', stderr);
                return false;
            }
        }

        if (std::ferror(infile))
        {
            fputs(CMDNAME ": Error: Could not read. Abort!\n", stderr);
            return false;
        }

        if (std::feof(infile))
        {
            return true;
        }
    }
}

static bool
parse_as_chars(FILE * infile)
{
    if (0 == setvbuf(infile, NULL, _IONBF, 0))
        ; //DB std::cerr << CMDNAME ": Set stdin to unbuffered mode." << std::endl;

    char * buffy = new char[BUFFY_SIZE + 1];
    buffy[BUFFY_SIZE] = 0;
    
    bool gt_code = 0;
   
    bool good = true;
    bool work = true;
    while (work)
    {
        size_t count = 0;
        while (count < BUFFY_SIZE)
        {
            int ch = fgetc(infile);
            if (EOF == ch)
            {
                if (std::ferror(infile))
                {
                    fputs(CMDNAME ": Error: Could not read. Abort!\n", stderr);
                    good = false;
                }
                work = false;
                break;
            }
            buffy[count] = ch;
            ++count;
            if ('>' == ch) {
                ++gt_code;
                if (gt_code >= 2) break;
            }
        }

        if (count > 0)
        {
            // std::cerr << "Parse: '" << buffy << '\'' << std::endl;
            if (XML_STATUS_OK != XML_Parse(parser, buffy, count, count < BUFFY_SIZE))
            {
                if (block_curr <= block_end)
                {
                    fprintf(stderr, CMDNAME ": %s\n", XML_ErrorString(XML_GetErrorCode(parser)));
                    fputs(CMDNAME ": Error: Parsing Buffer. Abort!\n", stderr);
                }
                fputc('\n', stderr);
                good = false;
                work = false;
            }
        }
    }
    delete [] buffy;
    return good;
}

static int
run()
{
    parser = XML_ParserCreate(NULL);
    if (NULL == parser)
    {
        fputs(CMDNAME ": Error: Could not create expat parser. Abort!\n", stderr);
        return 1;
    }

    XML_SetElementHandler(parser, element_start, element_end);
    XML_SetDefaultHandler(parser, default_handler);
    XML_SetCommentHandler(parser, comment);

    bool ok = false;
    if (options_file.empty())
    {
        if (block_end != std::numeric_limits<unsigned>::max())
            ok = parse_as_chars(stdin);
        else
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
            fprintf(stderr, CMDNAME ": Error: Could not open input file '%s'. Abort!\n", options_file.c_str());
        }
    }
    
    XML_ParserFree(parser);

    if (block_start > element_count)
    {
        fprintf(stderr, CMDNAME ": Error: Block start out of range (%u). Abort!\n", element_count);
        return 1;
    }
              
    if (ok)
        fprintf(stderr,
                CMDNAME ": Number of Elements %u, Number of Found Elements  %u, Maximum Depth %u\n",
                element_count,
                element_found_count,
                element_depth_max);
                  
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
        fprintf(stderr, CMDNAME ": Error: %s\n", ex.what());
    }
    catch (...)
    {
        fputs(CMDNAME ": Error: Unknown Exception.\n", stderr);
    }
    return 1;
}

