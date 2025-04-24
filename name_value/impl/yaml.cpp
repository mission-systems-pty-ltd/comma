// This file is part of comma library
// Copyright (c) 2025 Vsevolod Vlaskine
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
// 4. Additionally, source code from this repository produced after 2022
//    must not be used in training or test datasets for training language
//    models and/or automated code generation
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

#include <yaml.h>
#include <iostream>
#include "../../base/exception.h"
#include "yaml.h"

namespace comma { namespace name_value { namespace impl { namespace yaml {

// static boost::property_tree::ptree xml_to_ptree_( boost::property_tree::ptree& ptree)
// {
//     boost::property_tree::ptree out= boost::property_tree::ptree();
//     boost::property_tree::ptree unnamed_array= boost::property_tree::ptree();
//     for ( boost::property_tree::ptree::iterator i=ptree.begin(); i!=ptree.end(); i++ )
//     {
//         //look ahead for duplicate name
//         boost::property_tree::ptree::iterator lah = i;
//         if ( ++lah != ptree.end() && i->first == lah->first )
//         {
//             //add to unnamed array
//             unnamed_array.push_back( std::make_pair( "", xml_to_ptree_( i->second ) ) );
//         }
//         else
//         {
//             if(unnamed_array.size()!=0)
//             {
//                 //assert((i-1)->first==i->first);
//                 //the previous_scalar of duplicated name
//                 unnamed_array.push_back( std::make_pair( "", xml_to_ptree_( i->second ) ) );
//                 out.add_child(i->first,unnamed_array);
//                 unnamed_array= boost::property_tree::ptree();
//             }
//             else
//             {
//                 out.add_child(i->first, xml_to_ptree_(i->second) );
//             }
//         }
//     }
//     out.put_value( comma::strip( ptree.get_value<std::string>(), " \n\t" ));
//     return out;
// }

static void parse( yaml_parser_t *parser, boost::property_tree::ptree& t, bool expecting_value = false, bool is_sequence = false )
{
    //COMMA_THROW( comma::exception, "implementing..." );
    std::cerr << "==> a" << std::endl; //std::cerr << "==> a: expecting_value: " << expecting_value << " is_sequence: " << is_sequence << std::endl;
    std::string scalar, previous_scalar;
    bool previous_was_scalar{false};
    std::pair< std::string, boost::property_tree::ptree > seq;
    while( true )
    {
        yaml_event_t event;
        yaml_parser_parse( parser, &event );
        auto event_type = event.type;
        scalar = event.type == YAML_SCALAR_EVENT ? std::string( reinterpret_cast< const char* >( event.data.scalar.value ) ) : "";
        yaml_event_delete( &event );
        switch( event_type )
        {
            case YAML_SCALAR_EVENT:
                std::cerr << "==> b: scalar: " << scalar << std::endl;
                if( is_sequence )
                {
                    std::cerr << "==> b.0: scalar: " << scalar << " previous_was_scalar: " << previous_was_scalar << std::endl;
                    if( previous_was_scalar ) { t.push_back( std::make_pair( "", boost::property_tree::ptree() ) )->second.put_value( previous_scalar ); }
                    previous_was_scalar = true;
                    previous_scalar = scalar;
                    std::cerr << "==> b.1: scalar: " << scalar << " previous_scalar: " << previous_scalar << std::endl;
                    break;
                }
                if( expecting_value )
                {
                    std::cerr << "==> b.2: put value: " << scalar << std::endl;
                    t.put_value( scalar );
                    return;
                }
                std::cerr << "==> b.3: add child: " << scalar << std::endl;
                parse( parser, scalar.empty() ? t : t.add_child( scalar, boost::property_tree::ptree() ), true );
                expecting_value = false;
                break;
            case YAML_SEQUENCE_START_EVENT:
                previous_was_scalar = false;
                std::cerr << "==> c: seq start" << std::endl; 
                parse( parser, t, false, true );
                break;
            case YAML_SEQUENCE_END_EVENT:
                if( previous_was_scalar ) { t.push_back( std::make_pair( "", boost::property_tree::ptree() ) )->second.put_value( previous_scalar ); }
                std::cerr << "==> d: seq end" << std::endl;
                return;
            case YAML_MAPPING_START_EVENT:
                previous_was_scalar = false;
                std::cerr << "==> e: map start" << std::endl;
                parse( parser, is_sequence ? t.add_child( previous_scalar, boost::property_tree::ptree() ) : t ); // todo
                return;
            case YAML_MAPPING_END_EVENT:
            case YAML_STREAM_END_EVENT:
            case YAML_DOCUMENT_END_EVENT:
            case YAML_NO_EVENT:
                std::cerr << "==> f: map end" << std::endl;
                return;
            case YAML_DOCUMENT_START_EVENT:
            case YAML_STREAM_START_EVENT:
            case YAML_ALIAS_EVENT:
                break; // todo? handle?
            // default:
            // {
            //     auto e = event.type;
            //     yaml_event_delete( &event );       
            //     COMMA_THROW( comma::exception, "expected yaml event type; got: " << e ); // never here?
            // }
        }
    }
}

boost::property_tree::ptree& to_ptree( const std::string& s, boost::property_tree::ptree& t )
{
    yaml_parser_t parser;
    yaml_parser_initialize( &parser );
    yaml_parser_set_input_string( &parser, reinterpret_cast< const unsigned char* >( &s[0] ), s.size() );
    parse( &parser, t );
    yaml_parser_delete( &parser );
    return t;
}

std::string from_ptree( const boost::property_tree::ptree& )
{
    std::string s;
    // todo
    return s;
}

} } } } // namespace comma { namespace name_value { namespace impl { namespace yaml {
