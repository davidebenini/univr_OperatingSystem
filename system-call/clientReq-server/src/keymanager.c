#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/types.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>

#include "../inc/keymanager.h"
#include "../inc/errExit.h"
#include "../inc/semaphore.h"

int shmid;
int semid;
struct Memoryrow *mempointer;
int *maxRowUsed;
const int FIXED_TIME = 60 * 5;				//variabile utilizzata per fissare 5 minuti di tempo

void alarmHandler(int sig) {
    if(sig == SIGALRM) {
        printf("<KeyManager: %d> Alarm scattato!\n", getpid());
	//Rimozione della chiave        
	enterInCriticalSection(semid);
        deleteInvalidKeys();
        exitFromCriticalSection(semid);
    }
}

int keymanager(int memoryId, int semaphoreId, struct Memoryrow *pointer) {
    shmid = memoryId;
    semid = semaphoreId;
    mempointer = pointer;

    if(signal(SIGALRM, alarmHandler) == SIG_ERR)		//in piu 
        errExit("Alarm handler failed");

    while(1) {
        alarm(30);
        pause();
    }
    
    return 0;
}

//Funzione chiamata solamente dal padre (server)
int insertKey(long key, char userCode[]) {
    enterInCriticalSection(semid);
    //Troviamo il primo slot libero disponibile
    int foundSlot = 0;
    for(int i=0; i<LENGTH_SHARED_MEM && foundSlot == 0; i++) {
        struct Memoryrow *row = &(mempointer[i]);
        if(row->key == 0) {
            strcpy(row->userCode, userCode);
            row->timestamp = time(NULL);        //tempo attuale
            row->key = key;
            printf("<Server> Dato inserito in memoria a posizione %i\n", i);
            foundSlot = 1;
            if((*maxRowUsed) <= i) {			//tengo traccia delle righe utilizzate
                (*maxRowUsed)++;						
            }
        }
    }
    exitFromCriticalSection(semid);
    return foundSlot;
}

//Funzione chiamata solo dal processo padre(server)
//Eliminazione delle rige con usercode vuoto e chiave = 0
//Le righe con usercode vuoto e chiave con valore > 0 sono chiavi già utilizzate
void deleteInvalidKeys() {
    time_t timestamp = time(NULL);            				//timestamp del sistema attuale					
    for(int i=0; i <(*maxRowUsed) && i< LENGTH_SHARED_MEM; i++) {	//maxrowUsed per non iterare tuteele 50 righe ma solo quele utilizzate	        
        struct Memoryrow *row = &(mempointer[i]);		   			
        //Individuo tutti i dati che non sono stati eliminati o sono vecchi
        int interval = (int) (timestamp - row->timestamp);			      
        if(row->key != 0 && interval > (int) (FIXED_TIME)) {		    
            printf("<KeyMananer> Dato scaduto(Usercode: %s key: %li) cancellato alla posizione %i\n", row->userCode, row->key, i);
            row->key=0;
            strcpy(row->userCode, "");
        }
    }
}















