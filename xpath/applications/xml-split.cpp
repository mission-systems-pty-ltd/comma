#include <cassert>

#include <iomanip>
#include <iostream>
#include <fstream>
#include <limits>
#include <list>
#include <set>
#include <sstream>
#include <string>

#include <expat.h>

#include <boost/filesystem.hpp>

#include <comma/application/command_line_options.h>
#include <comma/xpath/xpath.h>
#include <comma/xpath/applications/expat-util.h>

namespace FS = boost::filesystem;

#define CMDNAME "xml-split"

static std::string options_file;

static unsigned const BUFFY_SIZE = 1 * 1024 * 1024;

static unsigned block_end = 1000; 

// can't use list or map on xpath because of the < overloading
typedef std::set<comma::xpath, comma::xpath::less_t> exact_set_t;
static exact_set_t exact_set;
typedef std::list<std::string> grep_list_t;
static grep_list_t grep_list;

static std::list<comma::xpath> element_path_list;
static unsigned element_found = 0;
static signed element_found_index = 0;

class output_wrapper
{
public:
    output_wrapper();
    void set_name(std::string const & name);
    std::ostream & start();
    std::ostream & more() { return _destination; }
    
private:
    std::string _name;
    unsigned _block_count;
    unsigned _file_count;
    
    std::ofstream _destination;
};

static unsigned const TAG_MAX = 16;
static output_wrapper writers[TAG_MAX];

// ~~~~~~~~~~~~~~~~~~
// UTILITIES
// ~~~~~~~~~~~~~~~~~~
namespace comma
{
    // This needs to be in the comma namespace
    std::ostream &
    operator <<(std::ostream & os, comma::xpath const & p)
    {
        return p.output(os, '|');
    }
}

signed
grep(XML_Char const * element, comma::xpath const & element_path)
{
    // std::cerr << element << " or " << element_path << std::endl;

    signed idx = 0;
    {
        exact_set_t::const_iterator const end = exact_set.end();
        exact_set_t::const_iterator itr = exact_set.begin();
        for (; itr != end; ++itr, ++idx)
            if (*itr == element)
                return idx;
    }
    {
        grep_list_t::const_iterator const end = grep_list.end();
        grep_list_t::const_iterator itr = grep_list.begin();
        
        for (; itr != end; ++itr, ++idx)
            if (*itr == element)
                return idx;
    }
    return -1;
}

output_wrapper::output_wrapper()
: _name()
, _block_count(0)
{
    assert(NULL != this);
}
    
void
output_wrapper::set_name(std::string const & name)
{
    assert(NULL != this);
    assert(_name.empty());
    _name = name;
}
    
std::ostream &
output_wrapper::start()
{
    assert(NULL != this);
    assert(! _name.empty());

    ++_block_count;
    if (_block_count > block_end || ! _destination.good())
    {
        _destination.close();
        _block_count = 0;
    }
    
    if (! _destination.good())
    {
        std::ostringstream oss;
        oss << CMDNAME << "/" << _name;

        if (! FS::exists(oss.str()))
        {
            if (! FS::create_directory(oss.str()))
            {
                std::cerr << CMDNAME ": Error: Could not Create Output Directory '" << oss.str() << "'. Abort!" << std::endl;
                return _destination;
            }
            else
            {
                std::cerr << CMDNAME ": Create Output Directory '" << oss.str() << '\'' << std::endl;
            }
        }
        
        oss << "/" << std::setw(6) << std::setfill('0') << _file_count << ".xml";

        _destination.open(oss.str().c_str(), std::ios::out);
        if (! _destination.good())
        {
            std::cerr << CMDNAME ": Error: Could not open file '" << oss.str() << "'. Abort!" << std::endl;
            return _destination;
        }
        else
        {
            std::cerr << CMDNAME ": Create Output File '" << oss.str() << '\'' << std::endl;
        }

        ++_file_count;
    }
    return _destination;
}

// ~~~~~~~~~~~~~~~~~~
// USER INTERFACE
// ~~~~~~~~~~~~~~~~~~
static void XMLCALL
usage(bool const verbose)
{
    std::cerr <<   "Splits the file up into chunks based on the size, also does grep for efficiency"
                 "\nUSAGE:   " CMDNAME " [--limit=Q]  [--source=XMLFILE]"
                 "\nOPTIONS: --limit=Q; default 1000; to output just the elements between 1 and Q"
                 "\n         --source=XMLFILE to open and parse that file."
                 "\nRETURNS: 0 - on success"
                 "\n         1 - on data error; like invalid xml"
              << std::endl;
}

// ~~~~~~~~~~~~~~~~~~
// APPLICATION
// ~~~~~~~~~~~~~~~~~~
class xml_split_application : public simple_expat_application
{
public:
    xml_split_application();

protected:
    virtual void 
    do_default(XML_Char const * const str, int const length);

    virtual void
    do_element_start(char const * const element, char const * const * const attributes);

    virtual void
    do_element_end(char const * const element);
  
};

xml_split_application::xml_split_application()
: simple_expat_application(CMDNAME)
{
}

void 
xml_split_application::do_default(XML_Char const * const str, int const length)
{
    if (element_found_index >= 0)
        writers[element_found_index].more().write(str, length);
}

void
xml_split_application::do_element_start(char const * const element, char const * const * const attributes)
{
    // build the xpath
    comma::xpath element_path;
    if (! element_path_list.empty())
        element_path = element_path_list.back();
    element_path /= std::string(element);
    element_path_list.push_back(element_path);
    
    signed idx = grep(element, element_path);
    if (idx >= 0)
    {
        ++element_found_count;
        if (0 == element_found)
        {
            element_found_index = idx;
            writers[element_found_index].start();
        }
        ++element_found;
    }

    if (element_found > 0)
    {
        assert(element_found_index >= 0);
        std::ostream  & os(writers[element_found_index].more());
        os  << '<' << element << ' ';
        if (NULL != attributes)
            for (unsigned i = 0; NULL != attributes[i]; i += 2)
            {
                os << attributes[i] << "='";
                for (XML_Char const * p = attributes[i+1]; *p != 0; ++p)
                    if ('\'' == *p)
                        os << "&apos;";
                    else
                        os << *p;
                os << "' ";
            }
        os << '>';
    }
}

void
xml_split_application::do_element_end(char const * const element)
{
    comma::xpath const & element_path = element_path_list.back();
    bool const was_found = element_found > 0;

    signed idx = grep(element, element_path);
    if (idx >= 0)
        --element_found;
    
    if (was_found)
    {
        assert(element_found_index >= 0);
        writers[element_found_index].more() << "</" << element << '>';
        if (0 == element_found)
            writers[element_found_index].more() << std::endl;
    }
    
    if (0 == element_found)
        element_found_index = -1;

    element_path_list.pop_back();
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
            if ('-' == argv[i][0])
                continue;
            
            //std::string const str(argv[i]);
            if (0 == argv[i][1]) // length of 1
                exact_set.insert(comma::xpath(argv[i]));
            else if ('/' != argv[i][0])
                grep_list.push_back(argv[i]);
            else if ('/' != argv[i][1])
                exact_set.insert(comma::xpath(argv[i] + 1));
            else 
                grep_list.push_back(argv[i] + 2);
        }

        if (true)
        {
            std::ostream_iterator<comma::xpath const> out_itr(std::cerr, "\n");
            std::cerr << "Exacts ..." << std::endl;
            std::copy(exact_set.begin(), exact_set.end(), out_itr);
            std::cerr << "Partials ..." << std::endl;
            std::copy(grep_list.begin(), grep_list.end(), out_itr);
        }

        if (FS::exists(CMDNAME))
        {
            std::cerr << CMDNAME ": Error: Output Directory Name '" CMDNAME "' Already Exists on Filesystem. Abort!" << std::endl;
            return 1;
        }

        if (! FS::create_directory(CMDNAME))
        {
            std::cerr << CMDNAME ": Error: Could not Create Output Directory '" CMDNAME "' Already Exists on Filesystem. Abort!" << std::endl;
            return 1;
        }
        else
        {
            std::cerr << CMDNAME ": Create Output Directory '" CMDNAME "'" << std::endl;
        }
        
        signed idx = 0;
        {
            exact_set_t::const_iterator const end = exact_set.end();
            exact_set_t::const_iterator itr = exact_set.begin();
            for (; itr != end; ++itr, ++idx)
                writers[idx].set_name(itr->to_string('|'));
        }
        {
            grep_list_t::const_iterator const end = grep_list.end();
            grep_list_t::const_iterator itr = grep_list.begin();
            for (; itr != end; ++itr, ++idx)
                writers[idx].set_name(*itr);
        }
        
        xml_split_application app;
        
        return app.run(options_file);
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

