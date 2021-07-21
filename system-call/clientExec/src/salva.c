#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "../inc/errExit.h"
#include <string.h>


int main (int argc, char *argv[]) {
    printf("Benvenuto sono il programma salva!\n");
    if(argc < 3)
        errExit("Invalid arguments, no list arguments,passed, minimum 3 args");
    printf("Ora salvo i tuoi argomenti nel file %s\n", argv[1]); 
    
    int fileFD  = open(argv[1], O_CREAT | O_TRUNC | O_WRONLY,          
          S_IRGRP | S_IRUSR | S_IWGRP | S_IWUSR);
    if(fileFD == -1) {
        errExit("Error open/creation file");
    }
    

    for(int i=2; i<argc; i++) {                     
        write(fileFD, argv[i], strlen(argv[i]));
        write(fileFD, "\n", strlen("\n"));
    }
    if(close(fileFD) == -1)
        errExit("Error closing file");
    printf("Fine...\n");
    return 0;
}
