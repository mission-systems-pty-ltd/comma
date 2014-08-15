#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if ( argc < 2 ) {
        fprintf(stderr, "Usage: %s <seconds to sleep>\n\n", argv[0]);
        int alen = (int)strlen(argv[0]);
        fprintf(stderr, "%s is a verbose replacement of sleep (2) utility\n", argv[0]);
        fprintf(stderr, "%*c It will print a message at the end of the run, if terminates\n", alen, ' ');
        fprintf(stderr, "%*c normally, and stay silent if killed.\n", alen, ' ');
        fprintf(stderr, "%*c Input is an integer number of seconds to sleep.\n", alen, ' ');
        return 1;
    }
    int d = atoi( argv[1] );
    sleep( d );
    fprintf(stdout, "%s: normal exit from slumber\n", argv[0]);
    return 0;
}
