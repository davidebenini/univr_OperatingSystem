#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "../inc/errExit.h"
#include "../inc/sharedmemory.h"
#include "../inc/semaphore.h"
#include "../inc/constant.h"

const int SERVICE_PRINT = 1;
const int SERVICE_SAVE = 2;
const int SERVICE_SEND = 3;

const char *IPC_SHD_MEM_KEY_PATH = "../IPC_KEYS/ipc_key_mem.conf";
const char *IPC_SEM_KEY_PATH = "../IPC_KEYS/ipc_key_sem.conf";

int main (int argc, char *argv[]) {
    if(argc < 3) {
        errExit("Wrong arugments number, minimum 3 please...");
    }

    char *userCode = argv[1];                          
    long key = strtol(argv[2], NULL, 10);		

    //---SEMAFORO---//
    key_t semkey = ftok(IPC_SEM_KEY_PATH, 'a');
    if(semkey == -1) {
        errExit("Creation token for semaphore failed");
    }
    int semid = getSemaphore(semkey, 1);					
    //---MEMORIA CONDIVISA---//
    //Accedo alla memoria condivisa, attach, utilizzo dei semafori
    key_t memkey = ftok(IPC_SHD_MEM_KEY_PATH, 'a');
    if(memkey == -1) {
        errExit("Creation token for memory failed");
    }
    int shmid = getSharedMemory(memkey, sizeof(struct Memoryrow) * (int) LENGTH_SHARED_MEM);	//shared memory creata dal server
    struct Memoryrow *pointer = (struct Memoryrow*) attachSharedMemory(shmid, 0);		
    
    //find and mark key, verifica la validità della chiave
    enterInCriticalSection(semid);
    int findMark = findAndMark(userCode, key, pointer);
    exitFromCriticalSection(semid);
    
    freeSharedMemory(pointer);					//dopo aver verificato la validità della chiave libero la shdmem
    
    //Se troviamo una chiave
    if(findMark > 0) {
        printf("Chiave trovata e rimossa dalla memoria!\n");						
        int length = argc-2;           		//toglie nome programma e userCode chiave rimane per dopo      
        char *args[length+1];     			//per il nome del programma/servizio          
        int j = 3;
        for(int i=1; i<length; i++) {       
            args[i] = argv[j];
            j++;					//The program input parameter points to an executable.
							//List and array are always terminated with a NULL pointer.
							//By convention, the first item of argv is the name of the program.
							//All exec functions return no result on success.
        }
        args[length] = (char *)NULL;    		//di defalut null per exec da error                 
        //Riconosciamo il servizio dalla chiave
        int service = (int) (key % 10);	            			
        if(service == SERVICE_PRINT) {
            args[0] = "stampa";				//avvia exec sostiuisce il processo e viene scartato tutto
            execvp("./stampa", args);                   
        } else if(service == SERVICE_SEND) {
            args[0] = "invia";
            execvp("./invia", args);
        } else if(service == SERVICE_SAVE) {
            args[0] = "salva";
            execvp("./salva", args);
        } else {
            printf("Servizio inesistente. Errore con la chiave data...\n");
        }
        errExit("Errore nell'esecuzione del programma\n");
    } else {
        if(findMark == -1) {
            printf("La chiave richiesta è già stata utilizzata!\n");
        } else {
            printf("Coppia chiave, utente (%li, %s) inesistente.\n", key, userCode);
        }
    }
    
    return 0;
}
