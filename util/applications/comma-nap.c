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
