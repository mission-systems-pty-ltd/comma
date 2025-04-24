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

enum class on { none, scalar, seq, map };

static void parse( yaml_parser_t *parser, boost::property_tree::ptree& t, on what = on::none, bool is_name = false )
{
    //COMMA_THROW( comma::exception, "implementing..." );
    //std::cerr << "==> A" << std::endl; //std::cerr << "==> a: expecting_value: " << expecting_value << " is_sequence: " << is_sequence << std::endl;
    std::string scalar;
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
                switch( what )
                {
                    case on::none:
                    case on::scalar:
                        COMMA_THROW( comma::exception, "expected map or sequence, got scalar" );
                    case on::map:
                        if( is_name ) { parse( parser, t.add_child( scalar, boost::property_tree::ptree() ), on::map, false ); break; }
                        t.put_value( scalar );
                        return;
                    case on::seq:
                        t.push_back( std::make_pair( "", boost::property_tree::ptree() ) )->second.put_value( scalar );
                        break;
                }
                break;
            case YAML_SEQUENCE_START_EVENT:
                //std::cerr << "==> c: seq start" << std::endl;
                parse( parser, t, on::seq );
                return;
            case YAML_SEQUENCE_END_EVENT:
                //std::cerr << "==> d: seq end" << std::endl;
                return;
            case YAML_MAPPING_START_EVENT:
                //std::cerr << "==> e: map start" << std::endl;
                if( what == on::seq )
                {
                    parse( parser, t.push_back( std::make_pair( "", boost::property_tree::ptree() ) )->second, on::map, true );
                    break;
                }
                parse( parser, t, on::map, true );
                return;
            case YAML_MAPPING_END_EVENT:
                //std::cerr << "==> f: map end" << std::endl;
                return;
            case YAML_STREAM_END_EVENT:
            case YAML_DOCUMENT_END_EVENT:
            case YAML_NO_EVENT:
                //std::cerr << "==> f: stream/document end or no event" << std::endl;
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
