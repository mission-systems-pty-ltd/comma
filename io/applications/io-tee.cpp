#include <comma/application/command_line_options.h>
#include <iostream>
#include <cstdio>   // for popen()
#include <cstdlib>
#include <string>
#include <boost/array.hpp>

static const char *app_name = "io-tee";

static void show_usage()
{
    std::cerr << "Usage: " << app_name << " <output file> [-u|--unbuffered --] <command ...>\n";
}

static void show_help( bool verbose )
{
    show_usage();
    std::cerr
        << std::endl
        << "Similar to \"tee >( command ... > file )\" in bash, but ensures that the command is complete before continuing." << std::endl
        << std::endl
        << "Note that only single commands are supported; to run multiple commands (or a pipeline), put them inside a bash function:" << std::endl
        << "*** IMPORTANT *** use \"export -f function_name\" to make the function visible to " << app_name << "." << std::endl
        << "Remember that " << app_name << " will not have access to the unexported variables, so pass any required values as function arguments." << std::endl
        << std::endl
        << "If any options are used (such as --unbuffered), \"--\" must precede the command." << std::endl
        << std::endl
        << "Example 1:" << std::endl
        << "    cat input | io-tee outfile grep \"one two\" | grep \"three\" > outfile2" << std::endl
        << std::endl
        << "    - which is equivalent to:" << std::endl
        << std::endl
        << "    cat input | grep \"one two\" > outfile" << std::endl
        << "    cat input | grep \"three\" > outfile2" << std::endl
        << std::endl
        << "Example 2:" << std::endl
        << "    function do_something() { local pattern=$1; grep \"$pattern\"; }" << std::endl
        << "    export -f do_something" << std::endl
        << "    cat input | io-tee outfile do_something \"one two\" | grep \"three\" > outfile2" << std::endl
        << std::endl
        << "    - exactly the same as Example 1, but using a function (note the \"export -f\")." << std::endl
        << std::endl;
}

static bool file_is_writable( const std::string &filename )
{
    FILE *file = fopen( filename.c_str(), "w" );
    if ( file == NULL ) { return false; }
    fclose( file );
    return true;
}

static std::string escape_quotes( const char *s )
{
    std::string result;
    for ( ; *s; ++s )
    {
        if ( *s == '"' ) { result += "\\\""; }          // double quote becomes \"
        else if ( *s == '\'' ) { result += "'\\''"; }   // single quote becomes '\'' (since the whole command is single quoted)
        else { result += *s; }
    }
    return result;
}

int main( int argc, char **argv )
{
    try
    {
        int dashdash_pos = -1;
        for ( int n = 1; n < argc; ++n ) { if ( std::string( argv[n] ) == "--" ) { dashdash_pos = n; break; } }
        int end_of_options = ( dashdash_pos == -1 ? std::min( argc, 2 ) : dashdash_pos );

        // if "--" is present, options must precede it
        comma::command_line_options options( end_of_options, argv, show_help );
        if ( argc < 3 ) { show_usage(); exit( 1 ); }

        bool unbuffered = false;
        std::string outfile;
        if ( dashdash_pos == -1 )
        {
            outfile = argv[1];
            if ( outfile[0] == '-' ) { std::cerr << app_name << ": \"--\" must be present in order to use options; found: " << outfile << std::endl; exit( 1 ); }
        }
        else
        {
            static const char *valueless_options = "--unbuffered,-u";
            static const char *options_with_values = "";
            const std::vector< std::string >& unnamed = options.unnamed( valueless_options, options_with_values );
            if ( unnamed.size() != 1 )
            {
                std::cerr << app_name << ": expected a single filename somewhere before \"--\", but found " << ( unnamed.size() == 0 ? "none" : "more than one:" );
                for ( size_t n = 0; n < unnamed.size(); ++n ) { std::cerr << ' ' << unnamed[n]; }
                std::cerr << std::endl;
                exit( 1 );
            }
            outfile = unnamed[0];
            if ( outfile[0] == '-' ) { std::cerr << app_name << ": unexpected option " << outfile << std::endl; exit( 1 ); }
            unbuffered = options.exists( "--unbuffered,-u" );
            if ( unbuffered ) { std::cerr << app_name << ": --unbuffered: not implemented yet" << std::endl; exit( 1 ); }
        }

        if ( !file_is_writable( outfile ) ) { std::cerr << app_name << ": cannot write to " << outfile << std::endl; exit( 1 ); }

        // bash -c only takes a single argument, so put the whole command in single quotes, then double quote each individual argument
        std::string command = "bash -c '" + escape_quotes( argv[2] );
        for ( int n = 3; n < argc; ++n ) { command += " \""; command += escape_quotes( argv[n] ); command += "\""; }
        command += " > ";
        command += outfile;
        command += "'";
        // TODO: remove this debugging once everything is working
        std::cerr << app_name << ": command: " << command << std::endl;

        FILE *pipe = popen( &command[0], "w" );
        if ( pipe == NULL ) { std::cerr << app_name << ": failed to open pipe; command: " << command << std::endl; exit( 1 ); }
        int ch;
        
        // TODO: io-tee in comma/io/applications; cmake: build it only for linux (io-cat can be a good hint)
        // TODO: by default, do buffered reading (64K)
        // TODO: --unbuffered,-u
        // TODO: if options present, expect -- before the command
        // TODO: use std::cin, std::cout by default
        // TODO: add result of reading from stdin
        // TODO: add result of reading from stdin
        
        boost::array< char, 0xffff > buffer;
        if( unbuffered )
        {
            // TODO: comma::io::select::wait() with timeout (git grep select in comma and/or snark for an example)
            // TODO: check how many bytes are available (git grep io-cat for an example)
            // TODO: read minimum of exactly that number of bytes or 64K
            return 1;
        }
        while( std::cin.good() )
        {
            std::cin.read( &buffer[0], buffer.size() );
            if( std::cin.gcount() <= 0 ) { break; }
            std::cout.write( &buffer[0], std::size_t( std::cin.gcount() ) );
            //int r = ::fwrite( pipe, &buffer[0], std::size_t( std::cin.gcount() ) );
            int r = ::fwrite( &buffer[0], sizeof( char ), ::size_t( std::cin.gcount() ), pipe );
            if( r >= 0 ) { continue; } // TODO: check what ::write actually returns
            std::cerr << app_name << ": error on pipe: error " << r << std::endl; // TODO: output text for the error
        }
        // TODO: output error code and its textual description ::perror( result )
        if ( pclose( pipe ) != 0 ) { std::cerr << app_name << ": command failed: " << command << std::endl; exit( 1 ); }
        return 0;
        
        while ( ( ch = getc( stdin ) ) != EOF ) { putc( ch, stdout ); putc( ch, pipe ); }
        if ( pclose( pipe ) != 0 ) { std::cerr << app_name << ": command failed: " << command << std::endl; exit( 1 ); }
        return 0;
    }
    catch( std::exception& ex ) { std::cerr << app_name << ": " << ex.what() << std::endl; }
    catch( ... ) { std::cerr << app_name << ": unknown exception" << std::endl; }
    return 1;
}

