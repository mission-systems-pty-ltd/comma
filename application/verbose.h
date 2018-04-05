// This file is part of comma, a generic and flexible library
// Copyright (c) 2015 The University of Sydney
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

#pragma once

#include <iostream>

//prototype for comma::verbose
namespace comma {

//a pseudo ostream for outputing information to stderr
//there is no need to instantiate from this class; just use comma::verbose
//examples: 
//  comma::verbose << "hello!" << std::endl;
//  if (comma::verbose) { std::cerr << comma::verbose.app_name() << "info" << std::endl; }
class verbose_t
{
    bool enabled_;
    std::string app_name_;
    bool start_of_line;
public:
    verbose_t();
    void init(bool enabled, const std::string& argv0);
    void flush();
    operator bool () const;
    const std::string& app_name() const;
    template<typename T>
    verbose_t& operator<<(const T& t);
    verbose_t& operator<<(std::basic_ostream<char>& (*pf)(std::basic_ostream<char>&));
};

//this is defined and initialized by command_line_options
extern verbose_t verbose;

//implementation
template<typename T>
inline verbose_t& verbose_t::operator<<(const T& t)
{
    if(enabled_)
    {
        if(start_of_line)
        {
            std::cerr<<app_name_<<": ";
            start_of_line=false;
        }
        std::cerr<<t;
    }
    return *this;
}

}//namespace comma {
