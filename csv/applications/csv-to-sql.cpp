// This file is part of comma, a generic and flexible library
// Copyright (c) 2011 The University of Sydney
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

/// @author david fisher

#ifdef WIN32
#include <fcntl.h>
#include <io.h>
#endif

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <boost/unordered_set.hpp>
#include "../../application/command_line_options.h"
#include "../../csv/options.h"
#include "../../string/string.h"

static const char *app_name = "csv-to-sql";
static const char *default_null_value = "NULL";

static void usage( bool verbose )
{
    std::cerr << "Usage: " << app_name
        << " [-h|--help] -t|--table=<table name> --fields=<input fields> [--table-fields=<table fields>] [--null-value=<string>]" << std::endl;

    if ( verbose )
    {
        std::cerr
            << std::endl
            << "Converts csv input to MySQL \"insert\" statements." << std::endl
            << std::endl
            << "Options:" << std::endl
            << "    -h|--help           Show this help" << std::endl
            << "    -t|--table-name=    Table name to use in SQL statements" << std::endl
            << "    --fields=           Input fields (comma separated)" << std::endl
            << "    --table-fields=     Fields to insert into the table (by default, same as input fields)" << std::endl
            << "    --null-value=       Input value to treat as NULL (default \"" << default_null_value << "\")" << std::endl
            << std::endl
            << "Example:" << std::endl
            << "    echo 0,1,hello,,NULL | " << app_name << " -t=fred --fields=ignored,id,str,from,to --table-fields=str,id,from,to" << std::endl
            << std::endl
            << "Output:" << std::endl
            << "    insert into fred (str,id,fred.from,fred.to) values ('hello','1','',NULL);" << std::endl
            << std::endl;
    }

    exit( 1 );
}

struct field
{
    std::string name;
    unsigned int index;
    unsigned int offset;
    boost::optional< unsigned int > input_index;
    unsigned int input_offset;
    unsigned int size;
    field( const std::string& name, unsigned int index ) : name( name ), index( index ) {}
};

static boost::unordered_set< std::string > fill_reserved_words()
{
    static const char *reserved_words[] =
    {
        "access", "accessible", "add", "all", "alter", "analyze", "and", "any", "as", "asc",
        "asensitive", "audit", "before", "between", "bigint", "binary", "blob", "both", "by", "call",
        "cascade", "case", "change", "char", "character", "check", "cluster", "collate", "column",
        "column_value", "comment", "compress", "condition", "connect", "constraint", "continue",
        "convert", "create", "cross", "current", "current_date", "current_time", "current_timestamp",
        "current_user", "cursor", "database", "databases", "date", "day_hour", "day_microsecond",
        "day_minute", "day_second", "dec", "decimal", "declare", "default", "delayed", "delete", "desc",
        "describe", "deterministic", "distinct", "distinctrow", "div", "double", "drop", "dual",
        "each", "else", "elseif", "enclosed", "escaped", "exclusive", "exists", "exit", "explain",
        "false", "fetch", "file", "float", "float4", "float8", "for", "force", "foreign", "from",
        "fulltext", "grant", "group", "having", "high_priority", "hour_microsecond", "hour_minute",
        "hour_second", "identified", "if", "ignore", "immediate", "in", "increment", "index", "infile",
        "initial", "inner", "inout", "insensitive", "insert", "int", "int1", "int2", "int3", "int4",
        "int8", "integer", "intersect", "interval", "into", "is", "iterate", "join", "key", "keys",
        "kill", "leading", "leave", "left", "level", "like", "limit", "linear", "lines", "load",
        "localtime", "localtimestamp", "lock", "long", "longblob", "longtext", "loop", "low_priority",
        "master_ssl_verify_server_cert", "match", "maxextents", "maxvalue", "mediumblob", "mediumint",
        "mediumtext", "middleint", "minus", "minute_microsecond", "minute_second", "mlslabel", "mod",
        "mode", "modifies", "modify", "natural", "nested_table_id", "noaudit", "nocompress", "not",
        "nowait", "no_write_to_binlog", "number", "numeric", "of", "offline", "on", "online",
        "optimize", "option", "optionally", "or", "order", "out", "outer", "outfile", "pctfree",
        "precision", "primary", "prior", "privileges", "procedure", "public", "purge", "range",
        "raw", "read", "reads", "read_write", "real", "references", "regexp", "release", "rename",
        "repeat", "replace", "require", "resignal", "resource", "restrict", "return", "revoke", "right",
        "rlike", "row", "rowid", "rownum", "rows", "schema", "schemas", "second_microsecond", "select",
        "sensitive", "separator", "session", "set", "share", "show", "signal", "size", "smallint",
        "spatial", "specific", "sql", "sql_big_result", "sql_calc_found_rows", "sqlexception",
        "sql_small_result", "sqlstate", "sqlwarning", "ssl", "start", "starting", "straight_join",
        "successful", "synonym", "sysdate", "table", "terminated", "then", "tinyblob", "tinyint",
        "tinytext", "to", "trailing", "trigger", "true", "uid", "undo", "union", "unique", "unlock",
        "unsigned", "update", "usage", "use", "user", "using", "utc_date", "utc_time", "utc_timestamp",
        "validate", "values", "varbinary", "varchar", "varchar2", "varcharacter", "varying", "view",
        "when", "whenever", "where", "while", "with", "write", "xor", "year_month", "zerofill",
        NULL
    };
    boost::unordered_set< std::string > s;
    for( int n = 0; reserved_words[n]; s.insert( reserved_words[n++] ) );
    return s;
}

// returns true if the argument is a reserved word (in either MySQL or Oracle SQL)
bool sql_reserved_word( const std::string &word )
{
    static boost::unordered_set< std::string > reserved_words = fill_reserved_words();
    std::string w = word;
    std::transform( w.begin(), w.end(), w.begin(), ::tolower );
    return reserved_words.find( w ) != reserved_words.end();
}

int main( int ac, char** av )
{
    try
    {
        if ( ac == 1 ) { usage( false ); }
        comma::command_line_options options( ac, av );
        if( options.exists( "--help,-h" ) ) { usage( true ); }

        comma::csv::options csv( options );
        csv.full_xpath = false;
        if ( csv.binary() ) { std::cerr << app_name << ": binary not handled" << std::endl; return 1; }

        std::string null_value = default_null_value;
        if ( options.exists( "--null-value" ) ) { null_value =  options.value< std::string >( "--null-value", std::string() ); }

        std::string table_name = options.value< std::string >( "--table-name,-t" );
        std::vector< std::string > input_fields = comma::split( csv.fields, ',' );
        std::vector< std::string > output_fields;
        if ( !options.exists( "--table-fields" ) ) { output_fields = input_fields; }
        else { output_fields = comma::split( options.value< std::string >( "--table-fields" ), ',' ); }

        std::vector< field > fields;
        for( unsigned int i = 0; i < output_fields.size(); ++i )
        {
            if( output_fields[i].empty() ) { continue; }
            fields.push_back( field( output_fields[i], i ) );
        }
        if( fields.empty() ) { std::cerr << app_name << ": please define at least one table field" << std::endl; return 1; }
        for( unsigned int i = 0; i < input_fields.size(); ++i )
        {
            for( unsigned int j = 0; j < fields.size(); ++j )
            {
                if( fields[j].name != input_fields[i] ) { continue; }
                fields[j].input_index = i;
            }
        }
        for( unsigned int i = 0; i < fields.size(); ++i )
        {
            if( !fields[i].input_index ) { std::cerr << app_name << ": \"" << fields[i].name << "\" not found in input fields " << csv.fields << std::endl; return 1; }
        }
        while( std::cin.good() && !std::cin.eof() )
        {
            std::string line;
            std::getline( std::cin, line );
            if( !line.empty() && *line.rbegin() == '\r' ) { line = line.substr( 0, line.length() - 1 ); }
            if( line.empty() ) { continue; }

            std::cout << "insert into " << table_name << " (";
            for ( size_t f = 0; f < output_fields.size(); ++f)
            {
                if ( f != 0 ) { std::cout << ','; }
                // for reserved words like "to", print "table_name.to", etc. (works for MySQL as well as Oracle)
                if ( sql_reserved_word( output_fields[f] ) ) { std::cout << table_name << '.'; }
                std::cout << output_fields[f];
            }

            std::cout << ") values (";

            std::vector< std::string > v = comma::split( line, csv.delimiter );
            std::string delimiter;
            unsigned int previous_index = 0;
            for( unsigned int i = 0; i < fields.size(); ++i ) // quick and dirty
            {
                for( unsigned int k = previous_index; k < fields[i].index && k < v.size(); ++k )
                {
                    std::cout << delimiter << v[k];
                    delimiter = csv.delimiter;
                }
                previous_index = fields[i].index + 1;
                std::cout << delimiter;
                if ( *fields[i].input_index < v.size() )
                {
                    std::string value = v[ *fields[i].input_index ];
                    if ( value == null_value ) { std::cout << "NULL"; }
                    else { std::cout << "'" << value << "'"; }
                }
                delimiter = csv.delimiter;
            }
            std::cout << ");" << std::endl;
        }
        return 0;
    }
    catch( std::exception& ex )
    {
        std::cerr << app_name << ": " << ex.what() << std::endl;
    }
    catch( ... )
    {
        std::cerr << app_name << ": unknown exception" << std::endl;
    }
    return 1;
}
