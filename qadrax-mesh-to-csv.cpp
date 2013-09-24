#include <iostream>
#include <fstream>
#include <vector>

#include <boost/unordered_map.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <comma/application/command_line_options.h>
#include <comma/csv/stream.h>

#include <aero/geometry/traits.h>

struct free_flight_edge {
    acfr::aero::coordinates source;
    size_t source_id;
    acfr::aero::coordinates target;
    size_t target_id;
};

struct airways_edge {
    size_t source_id;
    size_t target_id;
};

struct vertex {
    acfr::aero::coordinates p;
    size_t id;
};

namespace comma { namespace visiting {

template < > struct traits< vertex >
{
    template < typename K, typename V > static void visit( const K&, vertex& t, V& v )
    {
        v.apply("position", t.p);
        v.apply("id", t.id);
    }

    template < typename K, typename V > static void visit( const K&, const vertex& t, V& v )
    {
        v.apply("position", t.p);
        v.apply("id", t.id);
    }
};

template < > struct traits< airways_edge >
{
    template < typename K, typename V > static void visit( const K&, airways_edge& t, V& v )
    {
        v.apply("source_id", t.source_id);
        v.apply("target_id", t.target_id);
    }

    template < typename K, typename V > static void visit( const K&, const airways_edge& t, V& v )
    {
        v.apply("source_id", t.source_id);
        v.apply("target_id", t.target_id);
    }
};

template < > struct traits< free_flight_edge >
{
    template < typename K, typename V > static void visit( const K&, free_flight_edge& t, V& v )
    {
        v.apply("source", t.source);
        v.apply("source_id", t.source_id);
        v.apply("target", t.target);
        v.apply("target_id", t.target_id);
    }

    template < typename K, typename V > static void visit( const K&, const free_flight_edge& t, V& v )
    {
        v.apply("source", t.source);
        v.apply("source_id", t.source_id);
        v.apply("target", t.target);
        v.apply("target_id", t.target_id);
    }
};

} } // namespace comma { namespace visiting {

int main(int argc, char** argv){

    comma::command_line_options options(argc, argv);

    std::string nodes_input = options.value<std::string>("--nodes-in");
    std::string edges_input = options.value<std::string>("--edges-in");

    std::string nodes_output = options.value<std::string>("--nodes-out");
    std::string edges_output = options.value<std::string>("--edges-out");

    std::map<std::string, vertex> map;

    // read nodes
    std::ifstream node_reader;
    node_reader.open(nodes_input);
    std::string line;
    size_t counter = 1;

    // write nodes
    std::ofstream node_writer;
    node_writer.open(nodes_output);
    if(!node_writer.good()){
        COMMA_THROW(comma::exception, "Failed to open nodes out file");
    }
    comma::csv::options csv_options(options);
    comma::csv::output_stream<vertex> node_stream(node_writer, csv_options);

    while(node_reader.good()){
        getline(node_reader, line);

        if(line.size() == 0){
            break;
        }

        std::vector<std::string> tokens;
        boost::split(tokens, line, boost::is_any_of(","));

        vertex v;
        v.p.latitude = boost::lexical_cast<double>(tokens[0]) * M_PI / 180.0;
        v.p.longitude = boost::lexical_cast<double>(tokens[1]) * M_PI / 180.0;
        v.id = counter;

        map.insert(std::pair<std::string, vertex>(tokens[2].substr(0, tokens[2].size()-1), v));

        std::cout << tokens[2] << std::endl;

        counter++;

        node_stream.write(v);
    }

    node_reader.close();
    node_writer.close();

    for ( std::map<std::string,vertex>::iterator it=map.begin(); it!=map.end(); ++it)
       std::cout << it->first << " => " << it->second.id << '\n';



    // read edges
    std::ifstream edge_reader;
    edge_reader.open(edges_input);

    // write edges
    std::ofstream edge_writer;
    edge_writer.open(edges_output);
    if(!edge_writer.good()){
        COMMA_THROW(comma::exception, "Failed to open edges out file");
    }

    comma::csv::output_stream<airways_edge> edge_stream(edge_writer, csv_options);

    while(edge_reader.good()){
        getline(edge_reader, line);

        if(line.size() == 0){
            break;
        }

        std::vector<std::string> tokens;
        boost::split(tokens, line, boost::is_any_of(","));

        if(map.find(tokens[1].substr(0, tokens[1].size()-1)) == map.end()){
            COMMA_THROW(comma::exception, "cannot find key");
        }

        if(map.find(tokens[0]) == map.end()){
            COMMA_THROW(comma::exception, "cannot find key");
        }

        airways_edge e;
        e.source_id = map[tokens[0]].id;
        e.target_id = map[tokens[1].substr(0, tokens[1].size()-1)].id;

        edge_stream.write(e);
    }

    edge_reader.close();
    edge_writer.close();
}
