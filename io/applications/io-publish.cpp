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


/// @author cedric wohlleber

#ifdef WIN32
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#else
#include <errno.h>
#include <unistd.h>
#include <sys/select.h>
#endif

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/scoped_ptr.hpp>
#include "../../application/contact_info.h"
#include "../../application/command_line_options.h"
#include "../../application/signal_flag.h"
#include "../../base/last_error.h"
#include "../../string/string.h"
#include "publish.h"

//#include <google/profiler.h>

static void usage( bool verbose = false )
{
    std::cerr << std::endl;
    std::cerr << "read from standard input and write to given outputs (files, sockets, named pipes):" << std::endl;
    std::cerr << std::endl;
    std::cerr << "- the data is only written to the outputs that are ready for writing" << std::endl;
    std::cerr << "- client can connect and disconnect at any time" << std::endl;
    std::cerr << "- only full packets are written" << std::endl;
    std::cerr << std::endl;
    std::cerr << "usage: io-publish [<options>] <outputs>" << std::endl;
    std::cerr << std::endl;
    std::cerr << "stream options" << std::endl;
    std::cerr << "    --number,-n: ascii line-based input; buffer up to n lines, default: 0" << std::endl;
    std::cerr << "    --size,-s: binary input; packet size" << std::endl;
    std::cerr << "    --buffer,-b: buffer size, default is 0" << std::endl;
    std::cerr << "    --multiplier,-m: multiplier for packet size, default is 1. The actual packet size will be m*s" << std::endl;
    std::cerr << "    --no-discard: if present, do blocking write to every open pipe" << std::endl;
    std::cerr << "    --no-flush: if present, do not flush the output stream (use on high bandwidth sources)" << std::endl;
    std::cerr << std::endl;
    std::cerr << "client options" << std::endl;
    std::cerr << "    --exit-on-no-clients,-e: once the last client disconnects, exit" << std::endl;
    std::cerr << "    --output-number-of-clients,--clients: output to stdout timestamped number of clients whenever it changes" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    attention: in the current implementation, the number of clients will be updated only on attemt to write a new record," << std::endl;
    std::cerr << "               i.e. output number of clients will not change if there are no new records on stdin, even if the actual" << std::endl;
    std::cerr << "               number of clients changes" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    known problem: io::ostream or at least boost::asio::ostream does not mark stream as bad, if one tries to write to" << std::endl;
    std::cerr << "                   it first time after stream has been closed; the stream is marked as bad only after writing to it second time" << std::endl;
    std::cerr << "                   this problem is pretty benign: the worst thing that happens is writing to a closed stream, which will not" << std::endl;
    std::cerr << "                   cause grief unless you specifically rely io-publish on exiting on no clients for a rarely sent heartbeat" << std::endl;
    std::cerr << std::endl;
    std::cerr << "output streams" << std::endl;
    std::cerr << "    tcp:<port>: e.g. tcp:1234" << std::endl;
    std::cerr << "    udp:<port>: e.g. udp:1234 (todo)" << std::endl;
    std::cerr << "    local:<name>: linux/unix local server socket e.g. local:./tmp/my_socket" << std::endl;
    std::cerr << "    <named pipe name>: named pipe, which will be re-opened, if client reconnects" << std::endl;
    std::cerr << "    <filename>: a regular file" << std::endl;
    std::cerr << std::endl;
    std::cerr << comma::contact_info << std::endl;
    std::cerr << std::endl;
    exit( 0 );
}

class number_of_clients
{
    public:
        number_of_clients( const comma::command_line_options& options
                         , const std::vector< std::string >& names
                         , const comma::io::applications::publish::publishers_t& publishers )
            : output_number_of_clients_( options.exists( "--output-number-of-clients,--clients" ) )
            , exit_on_no_clients_( options.exists( "--exit-on-no-clients,-e" ) )
            , publishers_( publishers )
            , size_( publishers.size(), 0 )
        {
            if( output_number_of_clients_ ) { for( unsigned int i = 0; i < names.size(); ++i ) { if( names[i] == "-" ) { std::cerr << "io-publish: '-' and --output-number-of-clients are incompatible, since both output to stdout" << std::endl; exit( 1 ); } } }
        }
        
        bool update()
        {
            if( !output_number_of_clients_ && exit_on_no_clients_ ) { return true; }
            bool changed = false;
            unsigned int total = 0;
            for( unsigned int i = 0; i < publishers_.size(); ++i )
            {
                total += publishers_[i].size();
                changed = changed || publishers_[i].size() != size_[i];
                size_[i] = publishers_[i].size();
            }
            if( !changed ) { return true; }
            if( output_number_of_clients_ ) // quick and dirty, not using comma::csv to avoid coupling of comma::io with comma::csv
            {
                std::cout << boost::posix_time::to_iso_string( boost::posix_time::microsec_clock::universal_time() );
                for( unsigned int i = 0; i < size_.size(); ++i ) { std::cout << ',' << size_[i]; }
                std::cout << std::endl;
            }
            if( exit_on_no_clients_ && total == 0 ) { std::cerr << "io-publish: the last client exited" << std::endl; return false; }
            return true;
        }
        
    private:
        bool output_number_of_clients_;
        bool exit_on_no_clients_;
        const comma::io::applications::publish::publishers_t& publishers_;
        std::vector< unsigned int > size_;
};

int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av, usage );
        const boost::array< comma::signal_flag::signals, 2 > signals = { { comma::signal_flag::sigint, comma::signal_flag::sigterm } };
        comma::signal_flag is_shutdown( signals );
        unsigned int n = options.value( "-n,--number", 0 );
        unsigned int packet_size = options.value( "-s,--size", 0 ) * options.value( "-m,--multiplier", 1 );
        unsigned int buffer_size = options.value( "-b,--buffer", 0 );
        bool discard = !options.exists( "--no-discard" );
        bool flush = !options.exists( "--no-flush" );
        bool binary = packet_size != 0;
        const std::vector< std::string >& names = options.unnamed( "--no-discard,--verbose,-v,--no-flush,--output-number-of-clients,--clients,--exit-on-no-clients,-e", "-.+" );
        if( names.empty() ) { std::cerr << "io-publish: please specify at least one stream ('-' for stdout)" << std::endl; return 1; }
        if( binary )
        {
            //ProfilerStart( "io-publish.prof" ); {
            comma::io::applications::publish publish( names, n, buffer_size, packet_size, discard );
            number_of_clients nc( options, names, publish.publishers() );
            while( !is_shutdown && publish.read_bytes() && nc.update() );
            //ProfilerStop(); }
        }
        else
        {
            comma::io::applications::publish publish( names, n, 1, 0, discard, flush );
            number_of_clients nc( options, names, publish.publishers() );
            while( !is_shutdown && std::cin.good() && !std::cin.eof() && publish.read_line() && nc.update() );
        }
        if( is_shutdown ) { std::cerr << "io-publish: interrupted by signal" << std::endl; }
        return 0;
    }
    catch( std::exception& ex )
    {
        #ifndef WIN32
        if( comma::last_error::value() == EINTR || comma::last_error::value() == EBADF ) { return 0; }
        #endif
        std::cerr << "io-publish: " << ex.what() << std::endl;
    }
    catch( ... )
    {
        #ifndef WIN32
        if( comma::last_error::value() == EINTR || comma::last_error::value() == EBADF ) { return 0; }
        #endif
        std::cerr << "io-publish: unknown exception" << std::endl;
    }
    return 1;
}
