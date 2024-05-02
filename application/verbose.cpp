// Copyright (c) 2015 The University of Sydney

#include "../io/impl/filesystem.h"
#include "../string/string.h"
#include "verbose.h"

namespace comma {

verbose_t verbose;
verbose_t::verbose_t():enabled_(false),app_name_(""),start_of_line(false) { }
void verbose_t::flush() { if(enabled_) { std::cerr.flush(); start_of_line=true; } }
verbose_t::operator bool () const {return enabled_;}
const std::string& verbose_t::app_name() const {return app_name_;}
void verbose_t::init(bool enabled, const std::string& argv0)
{
    if(!argv0.empty()) { app_name_ = comma::filesystem::path(argv0).filename().string(); } // comma::split( argv0, '/' ).back();
    enabled_=enabled;
    start_of_line=true;
}

verbose_t& verbose_t::operator<<(std::basic_ostream<char>& (*pf)(std::basic_ostream<char>&))
{
    if(enabled_)
    {
        pf(std::cerr);
         start_of_line=true;
    }
    return *this;
}

}//namespace comma {
