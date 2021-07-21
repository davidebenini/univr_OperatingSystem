#include <sys/shm.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>

#include "../inc/errExit.h"
#include "../inc/sharedmemory.h"

//Ottieniamo la memoria condivisa in caso sià già presente, return -1
int getSharedMemory(key_t key, size_t size) {
    int shmid = shmget(key, size, S_IWGRP | S_IRGRP | S_IRUSR | S_IWUSR);
    if(shmid == -1) {
        errExit("Creation memory failed");
    }
    return shmid;
}

//attach della memoria condivisa
void *attachSharedMemory(int shmid, int shmflg) {
    void *pointer = shmat(shmid, NULL, shmflg);
    if (pointer == (void *)-1)
        errExit("Attach shared memory failed");
    return pointer;
}

//detach della memoria condivisa
void freeSharedMemory(void *ptrSharedMemory) {
    if(shmdt(ptrSharedMemory) == -1)
        errExit("Dettach shared memory failed");
}

//Le chiavi con usercode vuoto e chiave > 0 sono chiavi già uilizzate
int findAndMark(char *usercode, long key, struct Memoryrow *ptrSharedMemory) {
    for(int i=0; i< LENGTH_SHARED_MEM; i++) {			
        struct Memoryrow *row = &(ptrSharedMemory[i]);

        if(row->key == key) {					//primo if per dire la chiave è quella , mi serve sia per il return 1 che 									per il return 0		
            if(strcmp(row->userCode, usercode) == 0) {		
                if(row->key == key) {				//secondo if di controllo della chiave				
                    strcpy(row->userCode, "");	
                    return 1;		//Se trovo chiave e usercode uguali ritorno 1, ovvero marco la chiave come utilizzata
                }
            } else {
                if(strcmp(row->userCode, "") == 0) {			//Se invece la chiave è quella ma l'userCode è vuoto
									// Allora la chiave esiste ma era già stata utilizzata
                    return -1;						
                }
            }
        }
    }
    return 0;					//se la chiave non esiste return 0
}
