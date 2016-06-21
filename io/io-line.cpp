#include <iostream>
#include <iomanip>
#include <string>
#include <cstdio>
#include <cstring>

int main( int argc, const char ** argv )
{
    if( argc == 1 ) { /* usage() ; */;  return 1; }
    
    if( 0 == std::strcmp( argv[1], "length" ) )
    {
        std::string buffy; buffy.reserve( 1024 * 1024 * 1024 );
        
        while( std::cin )
        {
            std::getline( std::cin, buffy );
            if( ! std::cin ) return 0;
            std::cout
                // << std::setfill('0') << std::setw(8) 
                <<  buffy.size() << ',' << buffy
                << std::endl;
        }
    }
    else if( 0 == std::strcmp( argv[1], "get" ) )
    {
        ::setvbuf( stdin, (char *)NULL, _IONBF, 0 );
        
        unsigned length;
        char ch;
        
        if( ! std::cin ) { std::cerr << "comma-line-get: notice: input stream was not good at start" << std::endl; return 0; }
        std::cin >> length;
        if( ! std::cin ) { std::cerr << "comma-line-get: notice: could not get a length" << std::endl; return 0; }
        std::cin.get(ch);
        if( ! std::cin ) { std::cerr << "comma-line-get: error: could not get delimiter" << std::endl; return 1; }
        if( ch != ',' ) { std::cerr << "comma-line-get: error: delimiter was incorrect " << ch << std::endl; return 1; }
        
        char * const buffy = new char[length + 1];
        
        if( length > 0 )
        {
            buffy[length] = 0;
            std::cin.get(buffy, length + 1 );
            if( ! std::cin ) { std::cerr << "comma-line-get: error: could not get rest of line - tried for " << length << std::endl; return 1; }
            unsigned const sz = std::cin.gcount();
            if( length != sz ) { std::cerr << "comma-line-get: error: got " << sz << " characters but expected " << length << std::endl; return 1; }
        }
        
        std::cin.get(ch);
        if( ! std::cin ) { std::cerr << "comma-line-get: error: could not get end of line" << std::endl; return 1; }
        if( ch != '\n' ) { std::cerr << "comma-line-get: error: end of line was incorrect " << ch << std::endl; return 1; }
        
        if( length > 0 )
        {
            std::cout << buffy;
        }
        std::cout << std::endl;
    }
    
    return 0;
}