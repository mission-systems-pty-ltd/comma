// Copyright (c) 2011 The University of Sydney
// Copyright (c) 2020 Vsevolod Vlaskine

/// @authors cedric wohlleber, vsevolod vlaskine, dave jennings

#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <deque>
#include <memory>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>
#include "../../application/command_line_options.h"
#include "../../application/signal_flag.h"
#include "../../base/last_error.h"
#include "../../io/file_descriptor.h"
#include "../../io/publisher.h"
#include "../../name_value/map.h"
#include "../../string/string.h"
#include "../../sync/synchronized.h"

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
    std::cerr << "general options" << std::endl;
    std::cerr << "    --help,-h: show this help" << std::endl;
    std::cerr << "    --verbose,-v: more output to stderr" << std::endl;
    std::cerr << std::endl;
    std::cerr << "stream options" << std::endl;
    std::cerr << "    --cache-size,--cache=<n>; default=0; number of cached records; if a new client connects, the" << std::endl;
    std::cerr << "                                         the cached records will be sent to it once connected (todo)" << std::endl;
    std::cerr << "    --size,-s: binary input; packet size" << std::endl;
    std::cerr << "    --multiplier,-m: multiplier for packet size, default is 1. The actual packet size will be m * s" << std::endl;
    std::cerr << "    --no-discard: if present, do blocking write to every open stream" << std::endl;
    std::cerr << "    --no-flush: if present, do not flush the output stream (use on high bandwidth sources)" << std::endl;
    std::cerr << "    --exec=[<command>]: read from <command> rather than stdin" << std::endl;
    std::cerr << "    -- [<command>]: alternate syntax for specifying a command (simplifies quoting)" << std::endl;
    std::cerr << "    --on-demand: only run <command> when a client is connected" << std::endl;
    std::cerr << std::endl;
    std::cerr << "client options" << std::endl;
    std::cerr << "    --exit-on-no-clients,-e: once the last client disconnects, exit" << std::endl;
    std::cerr << "    --output-number-of-clients,--clients: output to stdout timestamped number of clients whenever it changes" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    attention: in the current implementation, the number of clients will be" << std::endl;
    std::cerr << "               updated only on attempt to write a new record," << std::endl;
    std::cerr << "               i.e. output number of clients will not change if there are no new" << std::endl;
    std::cerr << "               records on stdin, even if the actual number of clients changes" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    known problems: io::ostream or at least boost::asio::ostream does not mark" << std::endl;
    std::cerr << "               stream as bad, if one tries to write to it first time after" << std::endl;
    std::cerr << "               stream has been closed; the stream is marked as bad only after" << std::endl;
    std::cerr << "               writing to it second time." << std::endl;
    std::cerr << "               This problem is pretty benign: the worst thing that happens is" << std::endl;
    std::cerr << "               writing to a closed stream, which will not cause grief unless you" << std::endl;
    std::cerr << "               specifically rely on io-publish exiting on no clients for a" << std::endl;
    std::cerr << "               rarely sent heartbeat." << std::endl;
    std::cerr << std::endl;
    std::cerr << "               io-publish will not be very responsive in counting clients for" << std::endl;
    std::cerr << "               low bandwidth streams. It immediately recognises new clients" << std::endl;
    std::cerr << "               but might take a while to notice that a client has gone." << std::endl;
    std::cerr << "               This affects --output-number-of-clients and --on-demand." << std::endl;
    std::cerr << std::endl;
    std::cerr << "output streams: <address>[,<options>]" << std::endl;
    std::cerr << "    <address>" << std::endl;
    std::cerr << "        tcp:<port>: e.g. tcp:1234" << std::endl;
    std::cerr << "        udp:<port>: e.g. udp:1234 (todo)" << std::endl;
    std::cerr << "        local:<name>: linux/unix local server socket e.g. local:./tmp/my_socket" << std::endl;
    std::cerr << "        <named pipe name>: named pipe, which will be re-opened, if client reconnects" << std::endl;
    std::cerr << "        <filename>: a regular file" << std::endl;
    std::cerr << "        -: stdout" << std::endl;
    std::cerr << "    <options>" << std::endl;
    std::cerr << "        primary (default): clients always can connect to the 'primary' stream" << std::endl;
    std::cerr << "        secondary: clients can connect to the 'secondary' stream, only if there are existing clients on a primary stream" << std::endl;
    std::cerr << "                   if a client connects to a 'primary' stream, 'secondary' streams will be opened" << std::endl;
    std::cerr << "                   if last client on a 'primary' stream disconnects, 'secondary' streams will be closed" << std::endl;
    std::cerr << "                   e.g: io-publish tcp:8888 'tcp:9999;secondary'" << std::endl;
    std::cerr << std::endl;
    std::cerr << "examples" << std::endl;
    std::cerr << "    cat data | io-publish tcp:1234 --size 100" << std::endl;
    std::cerr << "    io-publish tcp:1234 --size 24000 --on-demand --exec \"camera-cat arg1 arg2\"" << std::endl;
    std::cerr << "    io-publish tcp:1234 --size 24000 --on-demand -- camera-cat arg1 arg2" << std::endl;
    std::cerr << std::endl;
    exit( 0 );
}

class publish
{
    public:
        typedef comma::synchronized< std::vector< std::unique_ptr< comma::io::publisher > > > publishers_t;
        
        typedef publishers_t::scoped_transaction transaction_t;
        
        struct endpoint
        {
            std::string address;
            bool secondary;
            endpoint( const std::string& address = "", bool secondary = false ): address( address ), secondary( secondary ) {}
        };
        
        publish( const std::vector< std::string >& endpoints
               , unsigned int packet_size
               , bool discard
               , bool flush
               , bool output_number_of_clients
               , bool update_no_clients
               , unsigned int cache_size )
            : discard_( discard )
            , flush_( flush )
            , buffer_( packet_size, '\0' )
            , packet_size_( packet_size )
            , output_number_of_clients_( output_number_of_clients )
            , cache_size_( cache_size )
            , update_no_clients_( update_no_clients )
            , got_first_client_ever_( false )
            , sizes_( endpoints.size(), 0 )
            , num_clients_( 0 )
            , is_shutdown_( false )
        {
            bool has_primary_stream = false;
            for( unsigned int i = 0; i < endpoints.size(); ++i )
            {
                comma::name_value::map m( endpoints[i], "address", ';', '=' );
                bool secondary = !m.exists( "primary" ) && m.exists( "secondary" );
                endpoints_.push_back( endpoint( m.value< std::string >( "address" ), secondary ) ); // todo? quick and dirty; better usage semantics?
                if( !secondary ) { has_primary_stream = true; }
            }
            if( !has_primary_stream ) { std::cerr << "io-publish: please specify at least one primary stream" << std::endl; exit( 1 ); }
            struct sigaction new_action, old_action;
            new_action.sa_handler = SIG_IGN;
            sigemptyset( &new_action.sa_mask );
            sigaction( SIGPIPE, NULL, &old_action );
            sigaction( SIGPIPE, &new_action, NULL );
            transaction_t t( publishers_ );
            t->resize( endpoints.size() );
            for( std::size_t i = 0; i < endpoints.size(); ++i )
            {
                if( !endpoints_[i].secondary ) { ( *t )[i].reset( new comma::io::publisher( endpoints_[i].address, is_binary_() ? comma::io::mode::binary : comma::io::mode::ascii, !discard, flush ) ); }
            }
            acceptor_thread_.reset( new boost::thread( boost::bind( &publish::accept_, boost::ref( *this ))));
        }
        
        ~publish()
        {
            is_shutdown_ = true;
            acceptor_thread_->join();
            transaction_t t( publishers_ );
            for( std::size_t i = 0; i < t->size(); ++i ) { if( ( *t )[i] ) { ( *t )[i]->close(); } }
        }
        
        void disconnect_all()
        {
            transaction_t t( publishers_ );
            for( auto& p: *t ) { if( p ) { p->disconnect_all(); } }
            handle_sizes_( t ); // quick and dirty
        }
        
        bool read( std::istream& input )
        {
            if( is_binary_() )
            {
                input.read( &buffer_[0], buffer_.size() );
                if( input.gcount() < int( buffer_.size() ) || !input.good() ) { return false; }
            }
            else
            {
                std::getline( input, buffer_ );
                buffer_ += '\n';
                if( !input.good() ) { return false; }
            }
            transaction_t t( publishers_ );
            if( cache_size_ > 0 )
            {
                cache_.push_back( buffer_ );
                if( cache_.size() > cache_size_ ) { cache_.pop_front(); }
            }
            for( auto& p: *t ) { if( p ) { p->write( &buffer_[0], buffer_.size(), false ); } } // for( std::size_t i = 0; i < t->size(); ++i ) { if( ( *t )[i] ) { ( *t )[i]->write( &buffer_[0], buffer_.size(), false ); } }
            return handle_sizes_( t );
        }
        
        unsigned int num_clients() const { return num_clients_; }

    private:
        bool is_binary_() const { return packet_size_ > 0; }
        
        bool handle_sizes_( transaction_t& t ) // todo? why pass transaction? it doen not seem going out of scope at the point of call; remove?
        {
            if( !output_number_of_clients_ && !update_no_clients_ ) { return true; }
            unsigned int total = 0;
            bool changed = false;
            has_primary_clients_ = false;
            for( unsigned int i = 0; i < t->size(); ++i )
            {
                unsigned int size = ( *t )[i] ? ( *t )[i]->size() : 0;
                total += size;
                if( !endpoints_[i].secondary && size > 0 ) { has_primary_clients_ = true; }
                if( sizes_[i] == size ) { continue; }
                sizes_[i] = size;
                changed = true;
                num_clients_ = total;
            }
            if( !changed ) { return true; }
            if( output_number_of_clients_ )
            {
                std::cout << boost::posix_time::to_iso_string( boost::posix_time::microsec_clock::universal_time() );
                for( unsigned int i = 0; i < sizes_.size(); ++i ) { std::cout << ',' << sizes_[i]; }
                std::cout << std::endl;
            }
            if( update_no_clients_ )
            {
                if( total > 0 ) { got_first_client_ever_ = true; }
                else if( got_first_client_ever_ ) { comma::verbose << "the last client exited" << std::endl; return false; }
            }
            return true;
        }
        
        void accept_()
        {
            comma::io::select select;
            {
                transaction_t t( publishers_ );
                for( unsigned int i = 0; i < t->size(); ++i )
                {
                    if( !( *t )[i] ) { continue; }
                    if( ( *t )[i]->acceptor_file_descriptor() != comma::io::invalid_file_descriptor ) { select.read().add( ( *t )[i]->acceptor_file_descriptor() ); }
                }
            }
            while( !is_shutdown_ )
            {
                select.wait( boost::posix_time::millisec( 100 ) ); // todo? make timeout configurable?
                transaction_t t( publishers_ );
                for( unsigned int i = 0; i < t->size(); ++i )
                {
                    if( ( *t )[i] && select.read().ready( ( *t )[i]->acceptor_file_descriptor() ) )
                    {
                        const auto& streams = ( *t )[i]->accept();
                        for( unsigned int i = 0; i < cache_.size(); ++i )
                        {
                            for( auto s: streams )
                            { 
                                ( *s )->write( &buffer_[0], buffer_.size() );
                                if( flush_ ) { ( *s )->flush(); }
                            }
                        }
                    }
                }
                handle_sizes_( t );
                if( has_primary_clients_ )
                {
                    for( unsigned int i = 0; i < t->size(); ++i )
                    {
                        if( !endpoints_[i].secondary || ( *t )[i] ) { continue; }
                        ( *t )[i].reset( new comma::io::publisher( endpoints_[i].address, is_binary_() ? comma::io::mode::binary : comma::io::mode::ascii, !discard_, flush_ ) );
                        if( ( *t )[i]->acceptor_file_descriptor() != comma::io::invalid_file_descriptor ) { select.read().add( ( *t )[i]->acceptor_file_descriptor() ); }
                    }
                }
                else
                {
                    for( unsigned int i = 0; i < t->size(); ++i )
                    {
                        if( !endpoints_[i].secondary || !( *t )[i] ) { continue; }
                        select.read().remove( ( *t )[i]->acceptor_file_descriptor() );
                        ( *t )[i].reset();
                    }
                }
            }
        }
        
        std::vector< endpoint > endpoints_;
        bool discard_;
        bool flush_;
        publishers_t publishers_;
        std::string buffer_;
        unsigned int packet_size_;
        bool output_number_of_clients_;
        unsigned int cache_size_;
        bool update_no_clients_;
        bool got_first_client_ever_;
        std::vector< unsigned int > sizes_;
        bool has_primary_clients_;
        unsigned int num_clients_;
        boost::scoped_ptr< boost::thread > acceptor_thread_;
        bool is_shutdown_;
        std::deque< std::string > cache_;
};

class command
{
    public:
        command( const std::string& command ): command_( command ), child_pid_( -1 )
        {
            comma::verbose << "launching command: " << command << std::endl;
            int fd[2];
            if( ::pipe( fd ) == -1 ) { comma::last_error::to_exception( "couldn't open pipe" ); } // create a pipe to send the child stdout to the parent stdin
            fd_ = fd[0];
            pid_t pid = fork();
            if( pid == -1 ) { comma::last_error::to_exception( "failed to fork()" ); }
            if( pid == 0 )
            {
                ::setsid(); // make the child a process group leader
                while( ( dup2( fd[1], STDOUT_FILENO ) == -1 ) && ( errno == EINTR ) ) {} // connect pipe input to stdout in child
                ::close( fd[1] );     // no longer need fd[1], now that it's duped
                ::close( fd[0] );     // don't need pipe output in the child
                ::execlp( "bash", "bash", "-c", &command_[0], NULL );
                std::cerr << "io-publish: failed to exec child: errno " << comma::last_error::value() << " - " << comma::last_error::to_string() << std::endl;
                exit( 1 );
            }
            child_pid_ = pid;
            comma::verbose << "launched command with pid: " << pid << std::endl;
            ::close( STDIN_FILENO );
            ::close( fd[1] ); // don't need pipe input in the parent
        }
        
        int fd() const { return fd_; }

        ~command()
        {
            comma::verbose << "closing file descriptor " << fd_ << " for " << comma::split( command_ )[0] << "..." << std::endl;
            ::close( fd_ );
            comma::verbose << "sending SIGTERM to " << comma::split( command_ )[0] << " (pid " << child_pid_ << ")..." << std::endl;
            ::kill( -child_pid_, SIGTERM );
            comma::verbose << "waiting for pid " << child_pid_ << "..." << std::endl;
            if( ::waitpid( -child_pid_, NULL, 0 ) < 0 ) { comma::verbose << "warning: waiting for pid " << child_pid_ << " failed" << std::endl; }
            while( std::getchar() >= 0 ); // todo: lame, but select or c-style reading produce bizarre results; investigate sometime
            comma::verbose << "waiting for pid " << child_pid_ << " done" << std::endl;
        }

    private:
        std::string command_;
        pid_t child_pid_;
        int fd_;
};

int main( int ac, char** av )
{
    try
    {
        std::vector< std::string > head, tail;
        for( int i = 0; i < ac && std::string( "--" ) != av[i]; ++i ) { head.push_back( av[i] ); }
        for( int i = head.size() + 1; i < ac; ++i ) { tail.push_back( av[i] ); }
        comma::command_line_options options( head, usage );
        const std::vector< std::string >& names = options.unnamed( "--no-discard,--verbose,-v,--no-flush,--output-number-of-clients,--clients,--exit-on-no-clients,-e,--on-demand", "-.+" );
        if( names.empty() ) { std::cerr << "io-publish: please specify at least one stream; use '-' for stdout" << std::endl; return 1; }
        options.assert_mutually_exclusive( "--cache-size,--cache", "--on-demand" );
        const boost::array< comma::signal_flag::signals, 2 > signals = { { comma::signal_flag::sigint, comma::signal_flag::sigterm } };
        comma::signal_flag is_shutdown( signals );
        bool on_demand = options.exists( "--on-demand" );
        bool exit_on_no_clients = options.exists( "--exit-on-no-clients,-e" );
        publish p( names
                 , options.value( "-s,--size", 0 ) * options.value( "-m,--multiplier", 1 )
                 , !options.exists( "--no-discard" )
                 , !options.exists( "--no-flush" )
                 , options.exists( "--output-number-of-clients,--clients" )
                 , exit_on_no_clients || on_demand
                 , options.value( "--cache-size,--cache", 0 ) );
        std::string exec_command = options.value< std::string >( "--exec", "" );
        if( !tail.empty() )
        {
            if( !exec_command.empty() ) { std::cerr << "io-publish: expected either --exec or --, got both" << std::endl; return 1; }
            exec_command = comma::join( tail, ' ' );
        }
        //ProfilerStart( "io-publish.prof" ); {
        if( exec_command.empty() )
        {
            if( on_demand ) { std::cerr << "io-publish: got --on-demand; please specify --exec <command> or -- <command>, or remove --on-demand" << std::endl; return 1; }
            while( std::cin.good() && !is_shutdown ) { if( !p.read( std::cin ) && exit_on_no_clients ) { break; } }
        }
        else
        {
            bool done = false;
            int fd[2];
            if( ::pipe( fd ) == -1 ) { comma::last_error::to_exception( "couldn't open pipe" ); } // create a pipe to send the child stdout to the parent stdin
            while( !done && !is_shutdown )
            {
                if( on_demand && p.num_clients() == 0 ) { ::sleep( 0.1 ); continue; } // todo? make timeout configurable?
                comma::verbose << "number of clients: " << p.num_clients() << std::endl;
                command cmd( exec_command );
                typedef boost::iostreams::file_descriptor_source fd_t;
                boost::iostreams::stream< fd_t > is( fd_t( cmd.fd(), boost::iostreams::never_close_handle ) );
                while( is.good() && !is_shutdown && p.read( is ) );
                if( !on_demand ) { break; }
                p.disconnect_all();
            }
        }
        //ProfilerStop(); }
        if( is_shutdown ) { std::cerr << "io-publish: interrupted by signal" << std::endl; }
        return 0;
    }
    catch( std::exception& ex )
    {
        if( comma::last_error::value() == EINTR || comma::last_error::value() == EBADF ) { return 0; }
        std::cerr << "io-publish: " << ex.what() << std::endl;
    }
    catch( ... )
    {
        if( comma::last_error::value() == EINTR || comma::last_error::value() == EBADF ) { return 0; }
        std::cerr << "io-publish: unknown exception" << std::endl;
    }
    return 1;
}
