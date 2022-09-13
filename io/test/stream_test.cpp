// Copyright (c) 2011 The University of Sydney

#include <cstdio>
#include <fstream>
#include <gtest/gtest.h>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <boost/filesystem/operations.hpp>
#include "../load.h" // just to make sure it compiles
#include "../select.h"
#include "../stream.h"

TEST( io, file_stream )
{
    {
        boost::filesystem::remove( "./test.pipe" );
        boost::filesystem::remove( "./test.file" );
        comma::io::ostream ostream( "./test.file" );
        comma::io::istream istream( "./test.file" );
        std::string line;
        *ostream << "hello, world" << std::endl;
        ostream->flush();
        std::getline( *istream(), line );
        EXPECT_EQ( line, "hello, world" );
        ostream.close();
        istream.close();
        boost::filesystem::remove( "./test.file" );
    }
    // todo: more testing?
    EXPECT_EQ( system( "mkfifo test.pipe" ), 0 );
    EXPECT_TRUE( boost::filesystem::exists( "./test.pipe" ) );
    EXPECT_TRUE( !boost::filesystem::is_regular_file( "./test.pipe" ) );
    EXPECT_TRUE( ::open( "./test.pipe", O_RDONLY | O_NONBLOCK ) > 0 );
    comma::io::ostream os( "./test.pipe" );
    EXPECT_TRUE( os() != NULL );
    EXPECT_TRUE( os->good() );
    EXPECT_EQ( system( "rm ./test.pipe" ), 0 );
}

TEST( io, std_stream )
{
    comma::io::istream istream( "-" );
    comma::io::ostream ostream( "-" );
    istream.close();
    ostream.close();
    // todo: more testing
}

TEST( io, tcp_stream )
{
//     boost::asio::io_service service;
//     boost::asio::ip::tcp::resolver resolver( service );
//     //boost::asio::ip::tcp::resolver::query query( "localhost" );
//     boost::asio::ip::tcp::resolver::query query( "localhost", "80" );
//     boost::asio::ip::tcp::resolver::iterator it = resolver.resolve( query );
//     std::cerr << "---> address=" << it->endpoint().address() << std::endl;
//     std::cerr << "---> port=" << it->endpoint().port() << std::endl;
//     //boost::asio::ip::tcp::endpoint endpoint( it, 12345 );
//     boost::asio::ip::tcp::socket socket( service );
//     //socket.connect( endpoint );
//     std::cerr << "---> socket.is_open()=" << socket.is_open() << std::endl;

//     // asio tcp server sample code
//     boost::asio::io_service service;
//     boost::asio::ip::tcp::endpoint endpoint( boost::asio::ip::tcp::v4(), 12345 );
//     boost::asio::ip::tcp::acceptor acceptor( service, endpoint );
//     boost::asio::ip::tcp::iostream stream;
//     std::cerr << "testTcpstream(): accepting..." << std::endl;
//     acceptor.accept( *stream.rdbuf() );
//     std::cerr << "testTcpstream(): accepted" << std::endl;
//     std::string line;
//     
//     int fd = stream.rdbuf()->native();
//     std::cerr << "testTcpstream(): fd = " << fd << std::endl;
//     comma::io::select select;
//     select.read().add( fd );
//     while( !stream.eof() )
//     {
//         std::cerr << "testTcpstream(): selecting..." << std::endl;
//         select.wait();
//         std::cerr << "testTcpstream(): select returned" << std::endl;
//         if( !select.read().ready( fd ) ) { break; }
//         std::getline( stream, line );
//         std::cerr << "testTcpstream(): just read \"" << line << "\"" << std::endl;
//     }
}

TEST( io, local_stream )
{
    #ifndef WIN32
    {
        boost::filesystem::remove( "./test.localsocket" );
        boost::asio::local::stream_protocol::endpoint endpoint( "test.localsocket" );
        EXPECT_TRUE( !boost::asio::local::stream_protocol::iostream( endpoint ) );
        boost::asio::io_service service;
        boost::asio::local::stream_protocol::acceptor acceptor( service, endpoint );
        EXPECT_TRUE( bool( boost::asio::local::stream_protocol::iostream( endpoint ) ) );
        comma::io::istream istream( "./test.localsocket" );
        comma::io::ostream ostream( "./test.localsocket" );
        istream.close();
        ostream.close();
        acceptor.close();
        EXPECT_TRUE( !boost::asio::local::stream_protocol::iostream( endpoint ) );
        EXPECT_TRUE( !boost::filesystem::is_regular_file( "./test.localsocket" ) );
        boost::filesystem::remove( "./test.localsocket" );
    }
    {
        boost::filesystem::remove( "./test.file" );
        comma::io::ostream ostream( "./test.file" );
        ostream.close();
        boost::asio::io_service service;
        boost::asio::local::stream_protocol::endpoint endpoint( "test.file" );
        try { boost::asio::local::stream_protocol::acceptor acceptor( service, endpoint ); EXPECT_TRUE( false ); } catch( ... ) {}
        boost::filesystem::remove( "./test.file" );
    }
    #endif
}

int main( int argc, char* argv[] )
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

