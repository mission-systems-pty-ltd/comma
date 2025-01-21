// Copyright (c) 2011 The University of Sydney
// Copyright (c) 2020 Vsevolod Vlaskine

/// @authors cedric wohlleber, vsevolod vlaskine, dave jennings

#include "../../application/command_line_options.h"
#include "../../application/signal_flag.h"
#include "../../base/last_error.h"
#include "../../io/file_descriptor.h"
#include "../../io/publisher.h"
#include "../../io/impl/publish.h"
#include "../../io/select.h"
#include "../../name_value/map.h"
#include "../../string/string.h"
#include "../../sync/synchronized.h"

//#include <google/profiler.h>

static void usage( bool verbose = false )
{
    std::cerr << R"(
read from standard input and write to given outputs (files, sockets, named pipes):

- the data is only written to the outputs that are ready for writing
- client can connect and disconnect at any time
- only full packets are written

usage: io-publish [<options>] <outputs>

general options
    --help,-h: show this help
    --verbose,-v: more output to stderr

stream options
    --cache-size,--cache=<n>; default=0; number of cached records; if a new client connects, the
                                         the cached records will be sent to it once connected
    --size,-s: binary input; packet size
    --multiplier,-m: multiplier for packet size, default is 1. The actual packet size will be m * s
    --no-discard: if present, do blocking write to every open stream
    --no-flush: if present, do not flush the output stream (use on high bandwidth sources)
    --exec=[<command>]: read from <command> rather than stdin
    -- [<command>]: alternate syntax for specifying a command (simplifies quoting)
    --on-demand: only run <command> when a client is connected
    --timeout-read,--read-timeout=<seconds>; exit or disconnect if no input data
                                             for longer than <seconds>
                                             limitation: if an input packet is half-read
                                                         io-publish still will block on read
    --timeout-reconnect,--reconnect-on-read-timeout, only if --exec present
    --timeout-is-error; exit with error on timeout

client options
    --exit-on-no-clients,-e: once the last client disconnects, exit
    --output-number-of-clients,--clients: output to stdout timestamped number of clients whenever it changes

    attention: in the current implementation, the number of clients will be
               updated only on attempt to write a new record,
               i.e. output number of clients will not change if there are no new
               records on stdin, even if the actual number of clients changes

    known problems: io::ostream or at least boost::asio::ostream does not mark
               stream as bad, if one tries to write to it first time after
               stream has been closed; the stream is marked as bad only after
               writing to it second time.
               This problem is pretty benign: the worst thing that happens is
               writing to a closed stream, which will not cause grief unless you
               specifically rely on io-publish exiting on no clients for a
               rarely sent heartbeat.

               io-publish will not be very responsive in counting clients for
               low bandwidth streams. It immediately recognises new clients
               but might take a while to notice that a client has gone.
               This affects --output-number-of-clients and --on-demand.

output streams: <address>[;<options>]
    <address>
        tcp:<port>: e.g. tcp:1234
        udp:<port>: e.g. udp:1234 (todo)
        local:<name>: linux/unix local server socket e.g. local:./tmp/my_socket
        <named pipe name>: named pipe, which will be re-opened, if client reconnects
        <filename>: a regular file
        -: stdout
    <options>
        primary (default): clients always can connect to the 'primary' stream
        secondary: clients can connect to the 'secondary' stream, only if there are existing clients on a primary stream
                   if a client connects to a 'primary' stream, 'secondary' streams will be opened
                   if last client on a 'primary' stream disconnects, 'secondary' streams will be closed
                   e.g: io-publish tcp:8888 'tcp:9999;secondary'

examples
    cat data | io-publish tcp:1234 --size 100
    io-publish tcp:1234 --size 24000 --on-demand --exec \"camera-cat arg1 arg2\"
    io-publish tcp:1234 --size 24000 --on-demand -- camera-cat arg1 arg2
)";
    exit( 0 );
}

class command
{
    public:
        command( const std::string& command ): command_( command ), child_pid_( -1 )
        {
            comma::saymore() << "launching command: " << command << std::endl;
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
                comma::say() << "failed to exec child: errno " << comma::last_error::value() << " - " << comma::last_error::to_string() << std::endl;
                exit( 1 );
            }
            child_pid_ = pid;
            comma::saymore() << "launched command with pid: " << pid << std::endl;
            ::close( STDIN_FILENO );
            ::close( fd[1] ); // don't need pipe input in the parent
        }
        
        int fd() const { return fd_; }

        ~command()
        {
            comma::saymore() << "closing file descriptor " << fd_ << " for " << comma::split( command_ )[0] << "..." << std::endl;
            ::close( fd_ );
            comma::saymore() << "sending SIGTERM to " << comma::split( command_ )[0] << " (pid " << child_pid_ << ")..." << std::endl;
            ::kill( -child_pid_, SIGTERM );
            comma::saymore() << "waiting for pid " << child_pid_ << "..." << std::endl;
            if( ::waitpid( -child_pid_, NULL, 0 ) < 0 ) { comma::saymore() << "warning: waiting for pid " << child_pid_ << " failed" << std::endl; }
            while( std::getchar() >= 0 ); // todo: lame, but select or c-style reading produce bizarre results; investigate sometime
            comma::saymore() << "waiting for pid " << child_pid_ << " done" << std::endl;
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
        const std::vector< std::string >& names = options.unnamed( "--no-discard,--verbose,-v,--no-flush,--output-number-of-clients,--clients,--exit-on-no-clients,-e,--on-demand,--timeout-reconnect,--reconnect-on-read-timeout,--timeout-is-error", "-.+" );
        if( names.empty() ) { comma::say() << "please specify at least one stream; use '-' for stdout" << std::endl; return 1; }
        options.assert_mutually_exclusive( "--cache-size,--cache", "--on-demand" );
        const boost::array< comma::signal_flag::signals, 2 > signals = { { comma::signal_flag::sigint, comma::signal_flag::sigterm } };
        comma::signal_flag is_shutdown( signals );
        bool on_demand = options.exists( "--on-demand" );
        bool exit_on_no_clients = options.exists( "--exit-on-no-clients,-e" );
        std::string exec_command = options.value< std::string >( "--exec", "" );
        bool reconnect_on_read_timeout = options.exists( "--timeout-reconnect,--reconnect-on-read-timeout" );
        bool timeout_is_error = options.exists( "--timeout-is-error" );
        boost::optional< double > read_timeout = options.optional< double >( "--timeout-read,--read-timeout" );
        COMMA_ASSERT_BRIEF( !reconnect_on_read_timeout || read_timeout, "--reconnect-on-read-timeout requires --read-timeout <seconds>" );
        COMMA_ASSERT_BRIEF( !reconnect_on_read_timeout || !exec_command.empty(), "--reconnect-on-read-timeout requires --exec <command>" );
        COMMA_ASSERT_BRIEF( !timeout_is_error || read_timeout, "--timeout-is-error requires --read-timeout <seconds>" );
        unsigned int size = options.value( "-s,--size", 0 );
        comma::io::impl::publish p( names
                                  , size * options.value( "-m,--multiplier", 1 )
                                  , !options.exists( "--no-discard" )
                                  , !options.exists( "--no-flush" )
                                  , options.exists( "--output-number-of-clients,--clients" )
                                  , exit_on_no_clients || on_demand
                                  , options.value( "--cache-size,--cache", 0 ) );
        if( !tail.empty() )
        {
            COMMA_ASSERT_BRIEF( exec_command.empty(), "expected either --exec or --, got both" );
            exec_command = comma::join( tail, ' ' );
        }
        //ProfilerStart( "io-publish.prof" ); {
        if( exec_command.empty() )
        {
            COMMA_ASSERT_BRIEF( !on_demand, "got --on-demand; please specify --exec <command> or -- <command>, or remove --on-demand" );
            comma::io::select select;
            if( read_timeout ) { select.read().add( 0 ); }
            std::ios_base::sync_with_stdio( false ); // unsync to make rdbuf()->in_avail() working
            std::cin.tie( NULL ); // std::cin is tied to std::cout by default
            while( std::cin.good() && !is_shutdown )
            {
                if( read_timeout )
                {
                    auto available = std::cin.rdbuf()->in_avail();
                    if( available == 0 ) // todo! || ( size > 0 && available < size ) )
                    {
                        select.wait( *read_timeout );
                        if( !select.read().ready( 0 ) )
                        {
                            comma::say() << "read: timeout no input after " << *read_timeout << " seconds" << std::endl;
                            exit( timeout_is_error ? 1 : 0 );
                        }
                    }
                }
                if( !p.read( std::cin ) && exit_on_no_clients ) { break; }
            }
        }
        else
        {
            COMMA_ASSERT_BRIEF( !read_timeout, "--read-timeout with --exec: implementing..." );
            bool done = false;
            int fd[2];
            if( ::pipe( fd ) == -1 ) { comma::last_error::to_exception( "couldn't open pipe" ); } // create a pipe to send the child stdout to the parent stdin
            while( !done && !is_shutdown )
            {
                if( on_demand && p.num_clients() == 0 ) { ::sleep( 0.1 ); continue; } // todo? make timeout configurable?
                comma::saymore() << "number of clients: " << p.num_clients() << std::endl;
                command cmd( exec_command );
                typedef boost::iostreams::file_descriptor_source fd_t;
                boost::iostreams::stream< fd_t > is( fd_t( cmd.fd(), boost::iostreams::never_close_handle ) );
                while( is.good() && !is_shutdown && p.read( is ) );
                if( !on_demand ) { break; }
                p.disconnect_all();
            }
        }
        //ProfilerStop(); }
        if( is_shutdown ) { comma::say() << "interrupted by signal" << std::endl; }
        return 0;
    }
    catch( std::exception& ex )
    {
        if( comma::last_error::value() == EINTR || comma::last_error::value() == EBADF ) { return 0; }
        comma::say() << "" << ex.what() << std::endl;
    }
    catch( ... )
    {
        if( comma::last_error::value() == EINTR || comma::last_error::value() == EBADF ) { return 0; }
        comma::say() << "unknown exception" << std::endl;
    }
    return 1;
}
