#include <stdlib.h>
#include <stdio.h>

#include "../inc/errExit.h"

int main (int argc, char *argv[]) {
    printf("Benvenuto sono il programma stampa!\n");
    if(argc < 2)
        errExit("No list arguments has given");

    for(int i=1; i<argc; i++) {
        printf("%s\n", argv[i]);
    }
    printf("Fine...\n");
    return 0;
}
