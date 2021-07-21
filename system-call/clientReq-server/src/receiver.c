#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include "./errExit.h"

const char *IPC_MSGQUEUE_KEY_PATH = "../IPC_KEYS/ipc_key_msg.conf";

struct Message {
    long type; /* Message type */
    char mtext[100]; /* Message body */
};

int main(){
    key_t queueKey = ftok(IPC_MSGQUEUE_KEY_PATH, 'a');
    if(queueKey == -1)
        errExit("Ftok per il receiver fallita!");
    printf("Benvenuto sono il programma receiver!\n");   
    printf("%i\n", queueKey);
    printf("Aspetto di ricevere un messaggio...\n");
    int msqid = msgget(queueKey, S_IRUSR | S_IWUSR | IPC_CREAT | IPC_EXCL);
    if(msqid == -1)        
	errExit("msgget fallita");
    struct Message received;
    if(msgrcv(msqid, &received, sizeof(struct Message) - sizeof(long), 1, 0) == -1)
        errExit("msgrcv fallita");
    printf("Ecco il messaggio: %s\n", received.mtext);
    if((msgctl(msqid, IPC_RMID, NULL)) == -1){
        errExit("Errore nella chiusura della coda di messaggi");
    }
}
