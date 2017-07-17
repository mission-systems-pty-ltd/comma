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

/// @author dewey nguyen

#ifdef WIN32
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#endif

#include <iostream>
#include "../../application/command_line_options.h"
#include "../../application/contact_info.h"

using namespace comma;

static void usage()
{
    std::cerr << std::endl;
    std::cerr << "Perform reshaping operations on input data" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Usage: cat data.csv | csv-reshape <operation> [options]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "   operations" << std::endl;
    std::cerr << "      concatenate: input lines to make longer output lines, e.g. 3 csv input lines to one output line" << std::endl;
    std::cerr << "          cat file.csv | csv-reshape concatenate -n 4 --discard" << std::endl;
    std::cerr << "      transpose: transpose rows into columns" << std::endl;
    std::cerr << "          cat file.bin | csv-reshape transpose --fields ,,block --binary t,d,ui" << std::endl;
    std::cerr << "options" << std::endl;
    std::cerr << "   general" << std::endl;
    std::cerr << "      --help,-h;  see this usage message" << std::endl;
    std::cerr << "      --verbose,-v: more output to stderr" << std::endl;
    std::cerr << "   concatenate" << std::endl;
    std::cerr << "      --delimiter,-d=<char>; default=','; field separating character" << std::endl;
    std::cerr << "      --discard; throw away last lines that are less than --lines" << std::endl;
    std::cerr << "      --lines,-n=<num>; number of input lines to concatenate into output lines" << std::endl;
    std::cerr << "      --sliding-window,-w; concatenate current input lines a the sliding window to create output lines" << std::endl;
    std::cerr << std::endl;
    std::cerr << comma::contact_info << std::endl;
    std::cerr << std::endl;
    exit( -1 );
}

int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av, usage );
        std::vector< std::string > unnamed = options.unnamed( "--sliding-window,-w,--discard,--help,-h,--verbose,-v", "--binary,-b,--delimiter,-d,--format,--fields,-f,--output-fields,--lines,-n" );
        comma::csv::options csv( options );
        #ifdef WIN32
        if( csv.binary() ) { _setmode( _fileno( stdin ), _O_BINARY ); _setmode( _fileno( stdout ), _O_BINARY ); }
        #endif
        if( unnamed.empty() ) { std::cerr << comma::verbose.app_name() << ": please specify operations" << std::endl; exit( 1 ); }
        std::vector< std::string > v = comma::split( unnamed[0], ',' );

        return 0;
    }
    catch( std::exception& ex ) { std::cerr << "csv-reshape: " << ex.what() << std::endl; }
    catch( ... ) { std::cerr << "csv-reshape: unknown exception" << std::endl; }
    usage();
}
