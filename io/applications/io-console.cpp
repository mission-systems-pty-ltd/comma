// Copyright (c) 2011 The University of Sydney

#include <termios.h>
#include <iostream>
#include <boost/program_options.hpp>
#include <boost/scoped_ptr.hpp>
#include "../../application/signal_flag.h"
#include "../select.h"

class no_echo_term
{
    public:
        no_echo_term()
        {
            ::tcgetattr( STDIN_FILENO, &_old );
            termios t = _old;
            t.c_lflag &= ~( ICANON | ECHO );
            ::tcsetattr( STDIN_FILENO, TCSANOW, &t );
        }

        ~no_echo_term() { ::tcsetattr( STDIN_FILENO, TCSANOW, &_old ); }

    private:
        struct termios _old;
};

int main( int argc, char** argv )
{
    try
    {
        double period;
        boost::program_options::options_description description( "options" );
        description.add_options()
            ( "help,h", "display help message" )
            ( "heartbeat,b", "output byte with value 0x00 when key pressed" )
            ( "period,t", boost::program_options::value< double >( &period )->default_value( 0.1, "0.1" ), "period in seconds between heartbeats" );
        boost::program_options::variables_map vm;
        boost::program_options::store( boost::program_options::parse_command_line( argc, argv, description), vm );
        boost::program_options::notify( vm );
        if ( vm.count( "help" ) )
        {
            std::cerr << std::endl;
            std::cerr << "take key presses, output key numeric values to stdout" << std::endl;
            std::cerr << "usage: io-console [<options>]" << std::endl;
            std::cerr << description << std::endl;
            std::cerr << std::endl;
            return 0;
        }
        boost::scoped_ptr< comma::io::select > select;
        unsigned int seconds{0}, nanoseconds{0};
        if ( vm.count( "heartbeat" ) )
        {
            select.reset( new comma::io::select() );
            select->read().add( 0 );
            seconds = std::floor( period );
            nanoseconds = std::floor( double( seconds == 0 ? period : std::fmod( period, seconds ) ) * 1e9 );
        }
        no_echo_term t;
        comma::signal_flag signal;
        while( !signal && std::cout.good() && !std::cout.bad() )
        {
            char c = 0x00;
            if( !select || select->wait( seconds, nanoseconds ) )
            {
                std::cin.read( &c, 1 );
                if( std::cin.gcount() <= 0 ) { break; }
            }
            std::cout.write( &c, 1 );
            std::cout.flush();
        }
        return 0;
    }
    catch( std::exception& ex ) { std::cerr << argv[0] << ": " << ex.what() << std::endl; }
    catch( ... ) { std::cerr << argv[0] << ": unknown exception" << std::endl; }
    return 1;
}
