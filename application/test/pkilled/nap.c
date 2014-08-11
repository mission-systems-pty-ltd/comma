#include <stdio.h>

int main(int argc, char *argv[]) {
    if ( argc < 2 ) {
        fprintf(stderr, "Usage: %s <seconds to sleep>\n", argv[0]);
        return 1;
    }
    int d = atoi( argv[1] );
    sleep( d );
    fprintf(stdout, "%s: normal exit from slumber\n", argv[0]);
    return 0;
}
