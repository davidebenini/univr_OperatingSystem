#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/stat.h>

#include "../inc/errExit.h"

struct Message {
    long type; 		//Tipo del messaggio
    char mtext[100]; 	//Corpo del messaggio
};


int main (int argc, char *argv[]) {
    printf("Benvenuto sono il programma Invia!\n");
    if(argc < 3)
        errExit("Invalid arguments, no list arguments,passed, minimum 3 args");
    int msgkey = atoi(argv[1]);
    if(msgkey < 0) {
        errExit("Invalid key");
    }
    int msgid = msgget(msgkey,  S_IRUSR | S_IWUSR);			//posso inviare su una esistente non crearla
    printf("Message key: %i\n", msgid);					//id della coda di messaggi
    if(msgid == -1) {
        errExit("Error open queue");
    }
    
    struct Message message;
    //Calcoliamo la lunghezza della stringa
    int length = 0;
    for(int i=3; i<argc; i++) {
        length += strlen(argv[i]);              
        length++; 						//piu lo spazio		       
    }

    //malloc passa il puntatore
    for(int i=3; i<argc; i++) {
        strcat(message.mtext, argv[i]);                   
        strcat(message.mtext, " ");                         
    }
    message.mtext[length-1] = '\0';
    message.type = (long) (argv[2]);
    printf("Tipo del messaggio: %li\n", message.type);                 

    if(msgsnd(msgid, &message, sizeof(message) - sizeof(long), 0) == -1)            
        errExit("Error sending message");
    //printf("Messaggio inviato: %s\nFine...\n", message.mtext);
    struct Message receive;
    if(msgrcv(msgid, &receive, sizeof(message) - sizeof(long), 1, 0) == -1)
	errExit("error"); 
    printf("Messaggio inviato: %s\nFine...\n", receive.mtext); 
    return 0;
}

