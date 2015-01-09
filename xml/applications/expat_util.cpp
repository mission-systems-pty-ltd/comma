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
// 3. All advertising materials mentioning features or use of this software
//    must display the following acknowledgement:
//    This product includes software developed by the University of Sydney.
// 4. Neither the name of the University of Sydney nor the
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

/// @author m-hounsell-acfr

#include <cassert>

#include <iostream>
#include <fstream>

#include "./expat_util.h"

static unsigned const BUFFY_SIZE = 1 * 1024; // * 1024;

// ~~~~~~~~~~~~~~~~~~
// SAX HANDLERS
// ~~~~~~~~~~~~~~~~~~
static void XMLCALL
handler_default(void * userdata, XML_Char const * str, int length)
{
    assert(NULL != userdata);
    assert(NULL != str);
    assert(length > 0);

    simple_expat_application * const ptr
        = reinterpret_cast<simple_expat_application *>(userdata);
        
    return ptr->default_handler(str, length);
}

static void XMLCALL
handler_comment(void * userdata, XML_Char const * str)
{
    // DO NOTHING
}

static void XMLCALL
handler_element_start(void * userdata, char const * element, char const ** attributes)
{
    assert(NULL != userdata);
    assert(NULL != element);
    assert(NULL != attributes);

    simple_expat_application * const ptr
        = reinterpret_cast<simple_expat_application *>(userdata);

    ptr->element_start(element, attributes);
}

static void
handler_element_end(void * userdata, char const * element)
{
    assert(NULL != userdata);
    assert(NULL != element);

    simple_expat_application * const ptr
        = reinterpret_cast<simple_expat_application *>(userdata);

    ptr->element_end(element);
}

// ~~~~~~~~~~~~~~~~~~
// CLASS
// ~~~~~~~~~~~~~~~~~~
simple_expat_application::simple_expat_application(char const * const name)
: command_name(name)
, parser(NULL)
, element_count(0)
, element_found_count(0)
, element_depth(0)
, element_depth_max(0)
{
}

int
simple_expat_application::run(std::string const & filename)
{
    parser = XML_ParserCreate(NULL);
    if (NULL == parser)
    {
        std::cerr << command_name << ": Error: Could not create expat parser. Abort!" << std::endl;
        return 1;
    }

    set_handlers();
    
    bool ok = false;
    if (filename.empty())
    {
        ok = parse_as_blocks(std::cin);
    }
    else
    {
        std::ifstream infile;
        infile.open(filename.c_str(), std::ios::in | std::ios::binary);
        if (infile.good())
        {
            ok = parse_as_blocks(infile);
            if (! ok)
                std::cerr << command_name << ": Error: Parsing Buffer. Abort!" << std::endl;
            infile.close();
        }
        else
        {
            std::cerr << command_name << ": Error: Could not open input file '" << filename.c_str() << "'. Abort!" << std::endl;
        }
    }
    
    XML_ParserFree(parser);
    
    std::cerr << command_name << ": Number of Elements " << element_count
              << ", Number of Found Elements " << element_found_count
              << ", Maximum Depth " << element_depth_max << std::endl;
                  
    return ok ? 0 : 1;
}

void
simple_expat_application::set_handlers()
{
    XML_SetElementHandler(parser, handler_element_start, handler_element_end);
    XML_SetDefaultHandler(parser, handler_default);
    XML_SetCommentHandler(parser, handler_comment);
    
    XML_SetUserData(parser, this);
}

bool
simple_expat_application::parse_retry_block(char const * const ptr, unsigned const size, bool const at_end)
{
    assert(NULL != ptr);
    assert('\0' != *ptr);
    assert(size > 0);
    
    long long byte_index = XML_GetCurrentByteIndex(parser);

    unsigned curr = 0;
    while (curr < size)
    {
        XML_ParserReset(parser, NULL);
        set_handlers();
        
        if (XML_STATUS_OK == XML_Parse(parser, ptr + curr, size - curr, at_end))
            return true;
        
        std::cerr << command_name << ": parse_retry_block L=" << XML_GetCurrentLineNumber(parser)
                << " C=" << XML_GetCurrentColumnNumber(parser)
                << " B=" << XML_GetCurrentByteIndex(parser);
        if (XML_ERROR_JUNK_AFTER_DOC_ELEMENT != XML_GetErrorCode(parser))
        {
            std::cerr << ": " << XML_ErrorString(XML_GetErrorCode(parser)) << std::endl;
            return false;
        }
        std::cerr << ": New Document Started" << std::endl;
        
        curr += XML_GetCurrentByteIndex(parser) - byte_index;
        byte_index = XML_GetCurrentByteIndex(parser);
    }
    return true;
}

bool
simple_expat_application::parse_as_blocks(std::istream & infile)
{
    assert(infile.good());
 
    long long read_byte_count = 0;
    for (;;)
    {
        void * const buffy = XML_GetBuffer(parser, BUFFY_SIZE);
        if (NULL == buffy)
        {
            std::cerr << command_name << ": Error: Could not allocate expat parser buffer. Abort!" << std::endl;
            return false;
        }

        infile.read(reinterpret_cast<char *>(buffy), BUFFY_SIZE);
        std::streamsize const gcount = infile.gcount();
        if (gcount > 0)
        {
            if (XML_STATUS_OK != XML_ParseBuffer(parser, gcount, gcount < BUFFY_SIZE))
            {
                std::cerr << command_name << ": parse_as_blocks L=" << XML_GetCurrentLineNumber(parser)
                        << " C=" << XML_GetCurrentColumnNumber(parser)
                        << " B=" << XML_GetCurrentByteIndex(parser);
                if (XML_ERROR_JUNK_AFTER_DOC_ELEMENT != XML_GetErrorCode(parser))
                {
                    std::cerr << ": " << XML_ErrorString(XML_GetErrorCode(parser)) << std::endl;
                    return false;
                }
                std::cerr << ": New Document Started" << std::endl;

                // only if we are trying to parse an xml stream
                long long const offset = XML_GetCurrentByteIndex(parser) - read_byte_count;
                if (! parse_retry_block(reinterpret_cast<char *>(buffy) + offset, gcount - offset, gcount < BUFFY_SIZE))
                    return false;
            }
            read_byte_count += infile.gcount();
        }

        if (infile.eof())
            return true;

        if (infile.bad())
        {
            std::cerr << command_name << ": Error: Could not read. Abort!" << std::endl;
            return false;
        }
    }
}

void 
simple_expat_application::default_handler(XML_Char const * const str, int const length)
{
    assert(NULL != this);

    return do_default(str, length);
}

void
simple_expat_application::element_start(char const * const element, char const * const * const attributes)
{
    assert(NULL != this);

    ++element_count;
    element_depth_max = std::max(element_depth_max, element_depth);
    ++element_depth;
    
    // build the xpath
    comma::xpath element_path;
    if (! element_path_list.empty())
        element_path = element_path_list.back();
    element_path /= std::string(element);
    element_path_list.push_back(element_path);

    do_element_start(element, attributes);
}

void
simple_expat_application::element_end(char const * const element)
{
    assert(NULL != this);

    do_element_end(element);

    element_path_list.pop_back();
    --element_depth;
}


