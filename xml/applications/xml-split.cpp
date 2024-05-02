// This file is part of comma, a generic and flexible library
// Copyright (c) 2011 The University of Sydney
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. Neither the name of the University of Sydney nor the
//    names of its contributors may be used to endorse or promote products
//    derived from this software without specific prior written permission.
//
// NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
// GRANTED BY THIS LICENSE.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
// HOLDERS AND CONTRIBUTORS \"AS IS\" AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
// BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
// IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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

#include "../../application/command_line_options.h"
#include "../../io/impl/filesystem.h"
#include "../../xpath/xpath.h"
#include "expat_util.h"

#define CMDNAME "xml-split"

static unsigned TOTAL_MAX = std::numeric_limits<unsigned>::max() - 1;

static unsigned block_limit = 1000; 
static unsigned total_limit = TOTAL_MAX; 

static bool options_verbose = false;
static std::string options_default_namespace = "";

// can't use list or map on xpath because of the < overloading
typedef std::set<comma::xpath, comma::xpath::less_t> exact_set_t;
static exact_set_t exact_set;
typedef std::list<std::string> grep_list_t;
static grep_list_t grep_list;

class output_wrapper
{
public:
    output_wrapper();
    void set_name(std::string const & name);
    bool is_full() const { return _total_count > total_limit; }
    std::ostream & start();
    std::ostream & more() { return _destination; }
    
private:
    std::string _folder;
    unsigned _block_count;
    unsigned _total_count;
    unsigned _file_count;
 
    bool _flag_open;   
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

static bool
is_all_full()
{
    unsigned const max = exact_set.size() + grep_list.size();
    assert(max <= TAG_MAX);
    unsigned full = 0;
    for (unsigned i = 0; i < max; ++i)
        if (writers[i].is_full()) ++full;
        
    return max == full;
}

static signed
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
    idx = exact_set.size();
    if (NULL == strchr(element, ':') && ! options_default_namespace.empty()) // no namespace
    {
        std::string renamed(options_default_namespace);
        renamed.append(":");
        renamed.append(element);
    
        grep_list_t::const_iterator const end = grep_list.end();
        grep_list_t::const_iterator itr = grep_list.begin();
        
        for (; itr != end; ++itr, ++idx)
            if (*itr == renamed)
                return idx;
    }
    
    return -1;
}

output_wrapper::output_wrapper()
: _folder()
, _block_count(0)
, _total_count(0)
, _file_count(0)
, _flag_open(false)
{
    assert(NULL != this);
}
    
void
output_wrapper::set_name(std::string const & name)
{
    assert(NULL != this);
    assert(_folder.empty());
    _folder = name;
    for (unsigned i = 0; i < _folder.size(); ++i) { if (':' == _folder[i]) { _folder[i] = '-'; } }
}
    
std::ostream &
output_wrapper::start()
{
    assert(NULL != this);
    assert(! _folder.empty());

    if (is_full())
    {
        if (_flag_open)
        {
            _destination.close(); 
            _flag_open = false;
        }
        return _destination;
    }

    std::ostringstream oss;
    oss << _folder;
    
    if (0 == _total_count)
    {
        if (comma::filesystem::exists(oss.str()))
        {
            std::cerr << CMDNAME ": Error: Output Directory Name '" << oss.str() << "' Already Exists on Filesystem. Abort!" << std::endl;
            exit(1);
        }
        else
        {
            if (! comma::filesystem::create_directory(oss.str()))
            {
                std::cerr << CMDNAME ": Error: Could not Create Output Directory '" << oss.str() << "'. Abort!" << std::endl;
                return _destination;
            }
        }
    }

    ++_block_count;
    ++_total_count;
    
    if (_block_count > block_limit || ! _destination.good())
    {
        _destination.close();
        _flag_open = false;
        _block_count = 0;
    }
    
    if (! _flag_open)
    {
        oss << "/" << std::setw(6) << std::setfill('0') << _file_count << ".xml";

        _destination.open(oss.str().c_str(), std::ios::out);
        if (! _destination.good())
        {
            std::cerr << CMDNAME ": Error: Could not open file '" << oss.str() << "'. Abort!" << std::endl;
            return _destination;
        }
        else if (options_verbose)
        {
            std::cerr << oss.str() << " ... " << std::flush;
        }

        _flag_open = true;
        ++_file_count;
    }

    return _destination;
}

// ~~~~~~~~~~~~~~~~~~
// USER INTERFACE
// ~~~~~~~~~~~~~~~~~~
static void XMLCALL usage(bool verbose)
{
    std::cerr << std::endl;
    std::cerr <<   "splits the file up into chunks based on the size, also does grep for efficiency" << std::endl;
    std::cerr << std::endl;
    std::cerr << "usage:   " CMDNAME " [--limit=Q]  [--source=XMLFILE]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options" << std::endl;
    std::cerr << "    --block=P; default 1000; to output just P elements per block" << std::endl
              << "    --default-namespace: specify a namespace to assume" << std::endl
              << "    --discard-namespace: (todo) discard namespace in the directory name" << std::endl
              << "                         e.g. both tags gml:id and id will be output to" << std::endl
              << "                         a directory called \"id\"" << std::endl
              << "                default: since some OSs don't support ':' in file/directory names, directories" << std::endl
              << "                         for tags with namespaces will have ':' replaced with '-'" << std::endl
              << "                         e.g: for the tag gml:id, the directory name will be gml-id" << std::endl
              << "    --total=Q; default INF; to output just Q of each element" << std::endl
              << "    --source=XMLFILE to open and parse that file." << std::endl
              << "    --verbose,-v: more output" << std::endl;
    std::cerr << std::endl;
    exit( 0 );
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

private:
    unsigned element_found_depth;
    signed element_found_index;
};

xml_split_application::xml_split_application()
: simple_expat_application(CMDNAME)
, element_found_depth(0)
, element_found_index(0)
{
}

void 
xml_split_application::do_default(XML_Char const * const str, int const length)
{
    if (element_found_index >= 0)
        if (! writers[element_found_index].is_full())
        {    
            writers[element_found_index].more().write(str, length);
        }
}

void
xml_split_application::do_element_start(char const * const element, char const * const * const attributes)
{
    comma::xpath const & element_path = current_xpath();
    
    signed idx = grep(element, element_path);
    if (idx >= 0)
    {
        ++element_found_count;
        if (0 == element_found_depth)
        {
            element_found_index = idx;
            writers[element_found_index].start();
        }
        ++element_found_depth;
    }
    
    if (element_found_depth > 0)
    {
        assert(element_found_index >= 0);
        if (! writers[element_found_index].is_full())
        {    
            std::ostream  & os(writers[element_found_index].more());
            os  << '<' << element;
            if (NULL != attributes)
                for (unsigned i = 0; NULL != attributes[i]; i += 2)
                {
                    os << ' ' << attributes[i] << "='";
                    for (XML_Char const * p = attributes[i+1]; *p != 0; ++p)
                        if ('\'' == *p)
                            os << "&apos;";
                        else
                            os << *p;
                    os << '\'';
                }
            os << '>';
        }
    }
}

void
xml_split_application::do_element_end(char const * const element)
{
    comma::xpath const & element_path = current_xpath();
    bool const was_found = element_found_depth > 0;

    signed idx = grep(element, element_path);
    if (idx >= 0)
        --element_found_depth;
    
    if (was_found)
    {
        assert(element_found_index >= 0);
        if (! writers[element_found_index].is_full())
        {    
            writers[element_found_index].more() << "</" << element << '>';
            if (0 == element_found_depth)
                writers[element_found_index].more() << std::endl;
        }
    }
    
    if (0 == element_found_depth)
        element_found_index = -1;
        
    if (is_all_full())
         XML_StopParser(parser, false);
}

// ~~~~~~~~~~~~~~~~~~
// MAIN
// ~~~~~~~~~~~~~~~~~~
int main(int argc, char ** argv)
{
    try
    {
        comma::command_line_options options( argc, argv, usage );
        options_verbose = options.exists( "--verbose,-v" );
        if (options.exists("--limit"))
        {
            std::cerr << CMDNAME ": Error: This option is OBSOLETE because of it's confusing name." << std::endl;
            return 1;
        }
        block_limit = options.value<unsigned>("--block", 1000);
        if (block_limit < 1)
        {
            std::cerr << CMDNAME ": Error: Block Limit must be greater then 1" << std::endl;
            return 1;
        }
        total_limit = options.value<unsigned>("--total", TOTAL_MAX);
        if (total_limit < 1)
        {
            std::cerr << CMDNAME ": Error: Total Limit must be greater then 1" << std::endl;
            return 1;
        }
        std::string options_file = options.value<std::string>("--source", "");
        options_default_namespace = options.value<std::string>("--default-namespace","");

        unsigned kept = 0;
        for (unsigned i = 1; i < unsigned(argc); ++i)
        {   
            if ('-' == argv[i][0]) { continue; }
            
            //std::string const str(argv[i]);
            if (0 == argv[i][1]) // length of 1
            {
                exact_set.insert(comma::xpath(argv[i]));
            }
            else if ('/' != argv[i][0])
            {
                grep_list.push_back(argv[i]);
            }
            else if ('/' != argv[i][1])
            {
                exact_set.insert(comma::xpath(argv[i] + 1));
            }
            else 
            {
                grep_list.push_back(argv[i] + 2);
            }
                
            ++kept;
            if (kept > TAG_MAX)
            {
                std::cerr << CMDNAME ": Error: Only " << TAG_MAX << " patterns are supported." << std::endl;
                return 1;
            }
        }
        
        if(grep_list.empty() && exact_set.empty()) { usage(true); return 1; }

        if (options_verbose)
        {
            std::ostream_iterator<comma::xpath const> out_itr(std::cerr, " ... ");
            if( options_verbose && ! grep_list.empty())
            {
                std::cerr << CMDNAME ": partial: ";
                std::copy(grep_list.begin(), grep_list.end(), out_itr);
                std::cerr << std::endl;
            }            
            if ( options_verbose && ! exact_set.empty())
            {
                std::cerr << CMDNAME ": exact: ";
                std::copy(exact_set.begin(), exact_set.end(), out_itr);
                std::cerr << std::endl;
            }
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
        
        if (options_verbose) { std::cerr << CMDNAME ": output: " << std::flush; }

        int const code = app.run(options_file);

        if (options_verbose) { std::cerr << std::endl; }
        
        return code;
    }
    catch (std::exception const & ex)
    {
        std::cerr << CMDNAME ": " << ex.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << CMDNAME ": unknown exception." << std::endl;
    }
    return 1;
}

