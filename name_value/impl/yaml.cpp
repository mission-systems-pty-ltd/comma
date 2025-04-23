#include <yaml.h>
#include <iostream>
#include "yaml.h"

namespace comma { namespace property_tree { namespace impl { namespace yaml {

enum storage_flags { VAR, VAL, SEQ };

void process_layer( yaml_parser_t *parser, unsigned int depth = 0 )
{
    yaml_event_t event;
    int storage = VAR; // mapping cannot start with VAL definition w/o VAR key
    while( true )
    {
        yaml_parser_parse( parser, &event );
        if( event.type == YAML_SCALAR_EVENT ) // parse value either as a new leaf in the mapping or as a leaf value (one of them, in case it's a sequence)
        {
            if( storage ) { std::cerr << "==> a: depth: " << depth << " scalar: event.data.scalar.value: " << event.data.scalar.value << std::endl; }
            else { std::cerr << "==> b: depth: " << depth << " scalar: event.data.scalar.value: " << event.data.scalar.value << std::endl; }
            storage ^= VAL; // flip VAR/VAL switch for the next event
        }
        else if ( event.type == YAML_SEQUENCE_START_EVENT) { std::cerr << "==> c: depth: " << depth << " sequence: start" << std::endl; storage = SEQ; } // sequence - all the following scalars will be appended to the last_leaf
        else if (event.type == YAML_SEQUENCE_END_EVENT) { std::cerr << "==> d: depth: " << depth << " sequence: end" << std::endl; storage = VAR; }
        else if ( event.type == YAML_MAPPING_START_EVENT )
        {
            std::cerr << "==> e: depth: " << depth << " mapping: start" << std::endl;
            process_layer( parser, depth + 1 );
            storage ^= VAL; // flip VAR/VAL, w/o touching SEQ
        }
        else if ( event.type == YAML_MAPPING_END_EVENT || event.type == YAML_STREAM_END_EVENT )
        {
            std::cerr << "==> e: depth: " << depth << " mapping: end" << std::endl;
            break;
        }
        yaml_event_delete( &event );
    }
}

boost::property_tree::ptree to_ptree( const std::string& s )
{
    boost::property_tree::ptree t;
    yaml_parser_t parser;
    yaml_parser_initialize( &parser );
    yaml_parser_set_input_string( &parser, reinterpret_cast< const unsigned char* >( &s[0] ), s.size() );
    process_layer( &parser );
    yaml_parser_delete( &parser );
    return t; // todo!!!
}

std::string from_ptree( const boost::property_tree::ptree& )
{
    std::string s;
    return s;
}

} } } } // namespace comma { namespace property_tree { namespace impl { namespace yaml {
