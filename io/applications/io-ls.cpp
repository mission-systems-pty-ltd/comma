#include <comma/application/command_line_options.h>
#include <comma/application/verbose.h>
#include <comma/csv/stream.h>
#include <comma/visiting/traits.h>
#include <boost/optional.hpp>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstring>
#include <arpa/inet.h>
#include <netinet/tcp.h>

boost::optional<unsigned int> port;
boost::optional<unsigned int> state;

/*
#include <stdio.h>          // printf, fopen, fclose
#include <stdlib.h>         // free

#define OK(x) ((x) > -1)
#define LISTENING (0x0A)
#define port u_int16_t

int listening(in_addr_t addr, port* ports, size_t nPorts) {

  char* line = NULL; size_t n = 0;
  u_int32_t locaddr; u_int32_t locport; u_int32_t state; int i = 0;
  FILE* tcp = fopen("/proc/net/tcp", "r");

  if   (!OK(getline(&line, &n, tcp))) goto f;
  while (OK(getline(&line, &n, tcp)) && i < nPorts) {
    int r = sscanf(line, "%*d: %8x:%4x %*8x:%*4x %2x", &locaddr, &locport, &state);
    if (r == 3 && addr == locaddr && state == LISTENING) ports[i++] = (port)locport;
  }

  f: fclose(tcp); free(line); return i;
}
*/

//tcp connection dump from /proc/net/tcp
//see: man proc 5
struct output_t
{
    unsigned int seq;
    struct address
    {
        unsigned int ip;
        unsigned int port;
    };
    address local;
    address remote;
    unsigned int state;
    unsigned int tx_queue;
    unsigned int rx_queue;
    unsigned int tr;
//     unsigned long tm_when;
    unsigned int retransmit;
    unsigned int uid;
    int timeout;
    void scan(const std::string& line)
    {
        unsigned long tm_when;
        //see: raw_sock_seq_show function for format http://lxr.oss.org.cn/ident?i=raw_sock_seq_show
        std::sscanf(line.c_str(), "%4d: %08X:%04X %08X:%04X"
            " %02X %08X:%08X %02X:%08lX %08X %5u %8d",  // "%lu %d %pK %d" inode
            &seq, &local.ip, &local.port, &remote.ip, &remote.port, 
            &state, &tx_queue, &rx_queue, &tr, &tm_when, &retransmit, &uid, &timeout);
    }
};

void write_state_map()
{
    std::cout << TCP_ESTABLISHED <<  ",ESTABLISHED" << std::endl;
    std::cout << TCP_SYN_SENT <<  ",SYN_SENT" << std::endl;
    std::cout << TCP_SYN_RECV <<  ",SYN_RECV" << std::endl;
    std::cout << TCP_FIN_WAIT1 <<  ",FIN_WAIT1" << std::endl;
    std::cout << TCP_FIN_WAIT2 <<  ",FIN_WAIT2" << std::endl;
    std::cout << TCP_TIME_WAIT <<  ",TIME_WAIT" << std::endl;
    std::cout << TCP_CLOSE <<  ",CLOSE" << std::endl;
    std::cout << TCP_CLOSE_WAIT <<  ",CLOSE_WAIT" << std::endl;
    std::cout << TCP_LAST_ACK <<  ",LAST_ACK" << std::endl;
    std::cout << TCP_LISTEN <<  ",LISTEN" << std::endl;
    std::cout << TCP_CLOSING <<  ",CLOSING" << std::endl;
}

std::string to_string(const in_addr& ipv4)
{
    std::string s(INET_ADDRSTRLEN, '\0');
    if(inet_ntop(AF_INET, &ipv4, &s[0], s.size()) == NULL) return "";
    s.resize(std::strlen(&s[0]));
    return s;
}

namespace comma { namespace visiting {

template <> struct traits< output_t::address >
{
    template< typename K, typename V > static void visit( const K& k, const output_t::address& t, V& v )
    {
        in_addr a;
        a.s_addr=t.ip;
        v.apply( "ip", to_string(a) );
        v.apply( "port", t.port );
    }
};

template <> struct traits< output_t >
{
    template< typename K, typename V > static void visit( const K& k, const output_t& t, V& v )
    {
        v.apply( "seq", t.seq );
        v.apply( "local", t.local );
        v.apply( "remote", t.remote );
        v.apply( "state",t.state );
        v.apply( "tx_queue", t.tx_queue );
        v.apply( "rx_queue", t.rx_queue );
        v.apply( "tr", t.tr );
//         v.apply( "tm_when", t.tm_when );
        v.apply( "retransmit", t.retransmit );
        v.apply( "uid", t.uid );
        v.apply( "timeout", t.timeout );
    }
};

} } // namespace comma { namespace visiting {
    
/*
int main0(int c, char* a[]) {
  struct in_addr ipv4; inet_pton(AF_INET, a[1], &ipv4);     printf("%i = ", ipv4.s_addr);
  char str[512];       inet_ntop(AF_INET, &ipv4, str, 512); printf("%s\n" , str);

  port pts[65536]; printf("{ listening: [ "); c = listening(ipv4.s_addr, pts, 65536);
  int i; for (i = 0; i < c; i++) printf("%i, ", pts[i]); printf("] }\n");
}
*/
void process_tcp(std::istream& is, const comma::csv::options& csv)
{
    std::string line;
    output_t output;
    comma::csv::output_stream<output_t> os(std::cout, csv);
    //first line is header
    std::getline(is,line);
    comma::verbose<<line<<std::endl;
    while(is.good())
    {
        std::getline(is,line);
        comma::verbose<<line<<std::endl;
        output.scan(line);
        if(port && output.local.port != *port) {continue;}
        if(state && output.state != *state) {continue;}
        os.write(output);
    }
}
void usage(bool detail)
{
    std::cerr << "    list tcp connections; reads and parses lines from /proc/net/tcp file and outputs them in csv format" << std::endl;
    std::cerr << std::endl;
    std::cerr << "usage:  " << comma::verbose.app_name() << " [ <options> ]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options" << std::endl;
    std::cerr << "    --help,-h: show help" << std::endl;
    std::cerr << "    --verbose,-v: show detailed messages" << std::endl;
    std::cerr << "    --output-fields: print field names to stdout" << std::endl;
    std::cerr << "    --output-format: print format to stdout, use --fields=<names> to get format of specific fields" << std::endl;;
    std::cerr << "    --enum-state: print state enumeration info as (numerical value,description) pairs" << std::endl;
    std::cerr << std::endl;
    if(detail)
    {
        std::cerr<< "csv options:" << std::endl;
        std::cerr<< comma::csv::options::usage() << std::endl;
        std::cerr<< std::endl;
    }
    else { std::cerr << "    see --help --verbose for more details" << std::endl<< std::endl; }
    std::cerr << "example" << std::endl;
    std::cerr << "    (" << comma::verbose.app_name() << " --output-fields; "<<comma::verbose.app_name() << ") | column -ts, " << std::endl;
    std::cerr << std::endl;
    std::cerr << "    io-ls --fields=local,state | csv-join --fields=,,state <(io-ls --enum-state)\";fields=state\"" << std::endl;
    std::cerr << std::endl;
    exit(0);
}
int main( int ac, char** av )
{
    comma::command_line_options options( ac, av, usage );
    try
    {
        comma::csv::options csv(options);
        csv.full_xpath=true;
        if(options.exists("--output-fields")) { std::cout<<comma::join(comma::csv::names<output_t>(), ',')<<std::endl; return 0;}
        if(options.exists("--output-format")) { std::cout<< comma::csv::format::value<output_t>(csv.fields,true)<<std::endl; return 0;}
        if(options.exists("--enum-state")) { write_state_map(); return 0;}
        port=options.optional<unsigned int>("--port");
        state=options.optional<unsigned int>("--state");
        std::fstream file("/proc/net/tcp", std::ios::in);
        process_tcp(file, csv);
    }
    catch( std::exception& ex )
    {
        std::cerr << comma::verbose.app_name() << ": " << ex.what() << std::endl; return 1;
    }
    catch( ... )
    {
        std::cerr << comma::verbose.app_name() << ": " << "unknown exception" << std::endl; return 1;
    }
    return 0;
}
