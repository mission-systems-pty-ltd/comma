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

namespace comma { namespace property_tree { namespace impl { namespace yaml {

static void parse( yaml_parser_t *parser, boost::property_tree::ptree& t )
{
    struct on { enum state { name = 0, value = 1, sequence = 2 }; };
    int state = on::name; // mapping cannot start with VAL definition w/o VAR key
    std::string name, value;
    while( true )
    {
        yaml_event_t event;
        yaml_parser_parse( parser, &event );
        switch( event.type )
        {
            case YAML_SCALAR_EVENT:
                ( state ? name : value ) = reinterpret_cast< const char* >( event.data.scalar.value );
                state ^= on::value; // flip on::name/on::value switch for the next event
                break;
            case YAML_SEQUENCE_START_EVENT:
                // todo: parse sequence
                state = on::sequence;
                break;
            case YAML_SEQUENCE_END_EVENT:
                // todo: end parsing sequence
                state = on::name;
                break;
            case YAML_MAPPING_START_EVENT:
            {
                // todo: create children
                auto q = t; // todo! pass child instead!
                parse( parser, t );
                state ^= on::value; // flip on::name/on::value switch for the next event
                break;
            }
            case YAML_MAPPING_END_EVENT:
            case YAML_STREAM_END_EVENT:
                break;
            default:
            {
                auto e = event.type;
                yaml_event_delete( &event );       
                COMMA_THROW( comma::exception, "expected yaml event type; got: " << e ); // never here?
            }
        }
        yaml_event_delete( &event );
    }
}

boost::property_tree::ptree& to_ptree( boost::property_tree::ptree& t, const std::string& s )
{
    yaml_parser_t parser;
    yaml_parser_initialize( &parser );
    yaml_parser_set_input_string( &parser, reinterpret_cast< const unsigned char* >( &s[0] ), s.size() );
    parse( &parser, t );
    yaml_parser_delete( &parser );
    return t;
}

boost::property_tree::ptree to_ptree( const std::string& s )
{
    boost::property_tree::ptree t;
    to_ptree( t, s );
    return t;
}

std::string from_ptree( const boost::property_tree::ptree& )
{
    std::string s;
    return s;
}

} } } } // namespace comma { namespace property_tree { namespace impl { namespace yaml {
