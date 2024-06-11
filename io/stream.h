// Copyright (c) 2011 The University of Sydney
// All rights reserved

/// @author vsevolod vlaskine

#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include "file_descriptor.h"

namespace comma { namespace io {

struct mode
{
    enum value { ascii = 0, binary = std::ios::binary };
    enum blocking_value { non_blocking = false, blocking = true };
};
    
/// interface class
/// constructs standard stream from name and owns it:
///     filename: file stream
///     -: std::cin or std::cout
///     tcp:address:port: tcp client socket stream
///     @todo udp:address:port: udp socket stream
///     @todo linux socket name: linux socket client stream
///     @todo serial device name: serial stream
/// see unit test for usage
template < typename S >
class stream : boost::noncopyable
{
    public:
        /// close stream, if closable
        void close();

        /// return pointer to stream
        S* operator()();

        /// return reference to stream
        S& operator*();

        /// return pointer to stream
        S* operator->();
        
        /// return pointer to stream
        const S* operator()() const;

        /// return reference to stream
        const S& operator*() const;

        /// return pointer to stream
        const S* operator->() const;

        /// @return file descriptor (to use in select)
        comma::io::file_descriptor fd() const;

        /// @return the number of characters available for reading on file descriptor
        /// @note for number of bytes available for reading in std::istream call rdbuf()->avail()
        /// e.g. std::cin.rdbuf()->in_avail()
        std::size_t available_on_file_descriptor() const;

        /// @return stream name
        const std::string& name() const;

        /// @return stream mode
        io::mode::value mode() const { return mode_; }

        /// @return true if stream is blocking
        bool blocking() const { return blocking_; }

    protected:
        stream( const std::string& name, mode::value mode, mode::blocking_value blocking );
        template < typename T >
        stream( T* s, io::file_descriptor fd, mode::value mode, mode::blocking_value blocking, boost::function< void() > close )
            : mode_( mode )
            , stream_( s )
            , close_( close )
            , fd_( fd )
            , close_d( false )
            , blocking_( blocking )
        {
        }
        ~stream();
        std::string name_;
        mode::value mode_;
        mutable S* stream_;
        mutable boost::function< void() > close_;
        comma::io::file_descriptor fd_;
        bool close_d;
        bool blocking_;
        S* lazily_make_stream_();
};
    
/// input stream owner
struct istream : public stream< std::istream >
{
    istream( const std::string& name, mode::value mode = mode::ascii, mode::blocking_value blocking = mode::blocking );
    istream( std::istream* s, io::file_descriptor fd, mode::value mode, boost::function< void() > close );
    istream( std::istream* s, io::file_descriptor fd, mode::value mode, mode::blocking_value blocking, boost::function< void() > close );
    static std::string usage( unsigned int indent = 0, bool verbose = false );
};

/// output stream owner
struct ostream : public stream< std::ostream >
{
    ostream( const std::string& name, mode::value mode = mode::ascii, mode::blocking_value blocking = mode::blocking );
    ostream( std::ostream* s, io::file_descriptor fd, mode::value mode, boost::function< void() > close );
    ostream( std::ostream* s, io::file_descriptor fd, mode::value mode, mode::blocking_value blocking, boost::function< void() > close );
    static std::string usage( unsigned int indent = 0, bool verbose = false );
};

/// input/output stream owner
struct iostream : public stream< std::iostream >
{
    iostream( const std::string& name, mode::value mode = mode::ascii, mode::blocking_value blocking = mode::blocking );
    static std::string usage( unsigned int indent = 0, bool verbose = false );
};

/// convenience class: multiple input streams read one by one
/// use case
///     - we have log files split by size, e.g. 1MB each: 0.bin, 1.bin, 2.bin, etc
///     - we want to read records from those files seamlessly
/// @todo currently, we assume a record never is split across two input files
///       support for split records: todo, just ask
/// @todo derive from std::istream (kinda super-fiddly, forwarding lots of methods...)
/// @todo support constructing from a directory name
class istreams
{
    public:
        // todo: istreams( const std::string& dir...
        istreams( const std::vector< std::string >& names, mode::value mode = mode::ascii, mode::blocking_value blocking = mode::blocking );
        static std::string usage( unsigned int indent = 0, bool verbose = false );
        bool eof() const { return _index + 1 < _names.size() || ( *_istream )->eof(); }
        bool read( char* buf, std::size_t size );
        std::string getline();
        void seek( std::uint64_t offset );
        stream< std::istream >& operator()() { return *_istream; }
        const stream< std::istream >& operator()() const { return *_istream; }
        istreams& operator++();
    protected:
        std::unique_ptr< istream > _istream;
        std::vector< std::string > _names;
        unsigned int _index{0};
        mode::value _mode;
        mode::blocking_value _blocking;
};

} } // namespace comma { namespace io {
