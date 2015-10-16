#pragma once

#include <iostream>
#include <boost/filesystem.hpp>
#include "command_line_options.h"

//prototype for comma::cverbose
namespace comma {

//a pseudo ostream for outputing information to stderr
//there is no need to instantiate from this class; just use comma::cverbose
//examples: 
//  comma::cverbose << "hello!" << std::endl;
//  if (comma::cverbose) { std::cerr << comma::cverbose.app_name() << "info" << std::endl; }
struct cverbose_t
{
    bool enabled;
    std::string app_name_;
    bool start_of_line;
public:
    void init(const command_line_options& options, const std::string& argv0);
    void flush();
    operator bool () const;
    const std::string& app_name() const;
    template<typename T>
    cverbose_t& operator<<(const T& t);
    cverbose_t& operator<<(std::basic_ostream<char>& (*pf)(std::basic_ostream<char>&));
};

//this is defined and initialized by command_line_options
extern cverbose_t cverbose;

//implementation
inline void cverbose_t::flush() { if(enabled) { std::cerr.flush(); start_of_line=true; } }
inline cverbose_t::operator bool () const {return enabled;}
inline const std::string& cverbose_t::app_name() const {return app_name_;}
inline void cverbose_t::init(const command_line_options& options, const std::string& argv0)
{
    if(!argv0.empty())
    {
        app_name_=boost::filesystem::basename(argv0);
    }
    enabled=options.exists("--verbose,-v");
    start_of_line=true;
}

template<typename T>
inline cverbose_t& cverbose_t::operator<<(const T& t)
{
    if(enabled)
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
inline cverbose_t& cverbose_t::operator<<(std::basic_ostream<char>& (*pf)(std::basic_ostream<char>&))
{
    if(enabled)
    {
        pf(std::cerr);
         start_of_line=true;
    }
    return *this;
}

}//namespace comma {
