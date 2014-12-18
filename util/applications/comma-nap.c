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
// 3. All advertising materials mentioning features or use of this software
//    must display the following acknowledgement:
//    This product includes software developed by the The University of Sydney.
// 4. Neither the name of the The University of Sydney nor the
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

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void usage(char *name, int brief) {
    fprintf(stderr, "Usage: %s <seconds to sleep> [-h|--help]\n", name);
    int alen = (int)strlen(name);
    if ( brief ) return;
    fprintf(stderr, "\n");
    fprintf(stderr, "%s is an auxiliary utility providing a verbose replacement of sleep (2).\n", name);
    fprintf(stderr, "%*c It prints a message at the end of a normal run, and stays silent if killed.\n", alen, ' ');
    fprintf(stderr, "%*c Thus, the user may detect if a kill signal reached its destination.\n", alen, ' ');
    fprintf(stderr, "%*c Input argument is an integer number of seconds to sleep.\n", alen, ' ');
}

int main(int argc, char *argv[]) {
    if ( argc < 2 ) {
        usage( argv[0], 1 );
        return 1;
    }
    if ( 0 == strcmp("-h", argv[1]) || 0 == strcmp("--help", argv[1]) ) {
        usage( argv[0], 0 );
        return 0;
    }
    char * endptr;
    int d = (int)( strtol( argv[1], &endptr, 10 ) );
    if ( *endptr != '\0' ) {
        fprintf(stderr, "%s: %s is not an integer\n\n", argv[0], argv[1]);
        usage( argv[0], 1 );
        return 1;
    }
    sleep( d );
    fprintf(stdout, "%s: normal exit from slumber\n", argv[0]);
    return 0;
}
