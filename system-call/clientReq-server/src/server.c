#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/ipc.h>

#include "../inc/errExit.h"
#include "../inc/keymanager.h"
#include "../inc/request_response.h"
#include "../inc/keygenerator.h"
#include "../inc/sharedmemory.h"
#include "../inc/semaphore.h"

//costanti utilizzate per memorizzare il servizio in un intero
const int SERVICE_PRINT = 1;			
const int SERVICE_SAVE = 2;		
const int SERVICE_SEND = 3;
const int NO_SERVICE = 9;

const char *IPC_SHD_MEM_KEY_PATH = "../IPC_KEYS/ipc_key_mem.conf";
const char *IPC_SEM_KEY_PATH = "../IPC_KEYS/ipc_key_sem.conf";

const char *serverFifoPath = "/tmp/server_fifo";
const char *baseClientFifoPath = "/tmp/client_fifo";
sigset_t signalset;
pid_t childpid;
int serverFifoFD;
int serverFifoExtraFD;
long requestNumber = 1;					

//
int shmid;
int indexPosShmid;
struct Memoryrow *mempointer;
int *maxRowUsed;		//variabile utilizzata per tenere traccia delle righe utilizzate nella shared memory

int semid;

//Funzione utilizzate per chiudere tutte le risorse utilizzate: fifo, semafori e memoria condivisa
void quit() {						
    if (serverFifoFD != 0 && close(serverFifoFD) == -1)
        errExit("close failed");

    if (serverFifoExtraFD != 0 && close(serverFifoExtraFD) == -1)
        errExit("close failed");

    if (unlink(serverFifoPath) != 0)
        errExit("unlink failed");
    
    freeSharedMemory(mempointer);
    removeSharedMemory(shmid);

    freeSharedMemory(maxRowUsed);
    removeSharedMemory(indexPosShmid);
    

    removeSemaphore(semid);
    
    _exit(0);
}

//handler per gestire la SIGTERM nel il mio set di segnali
void sigHandler(int sig) {				
    printf("Sigterm called %d\n", getpid());       	
    if(sig == SIGTERM) {
        printf("Killing process %d\n", childpid);
        if(kill(childpid, SIGTERM) == -1)
            printf("Killing child task failed!\n");
        wait(NULL);				//aspetta uccisione figlio
        quit();
    }
}

//funzione che mi ritorna il servizio corretto
const int getService(char serviceInput[]) {		
    if(strcmp(serviceInput, "Stampa") == 0) {
        return SERVICE_PRINT;
    } else if(strcmp(serviceInput, "Invia") == 0) {
        return SERVICE_SEND;
    } else if(strcmp(serviceInput, "Salva") == 0) {
        return SERVICE_SAVE;
    }
    return NO_SERVICE;
}

int main (int argc, char *argv[]) {

    printf("Processo server partito con pid: %d!\n", getpid());	
    //-----SEGNALI-----//
    //Creazione e inizializzazione dell'handler
    if(signal(SIGTERM, sigHandler) == SIG_ERR) {
        errExit("Error creation signal handler");
    }

    if(sigfillset(&signalset) == -1)
        errExit("Error filling signal set");
    if(sigdelset(&signalset, SIGTERM) == -1)
        errExit("Error removing signal from mask set");
    if(sigprocmask(SIG_SETMASK, &signalset, NULL) == -1)
        errExit("Error setting mask");

    //SHARED MEMORY
    indexPosShmid = createSharedMemoryFromSystem(sizeof(int));			
    maxRowUsed = attachSharedMemory(indexPosShmid, 0);
    *maxRowUsed = 0;						

    key_t keySharedMem = ftok(IPC_SHD_MEM_KEY_PATH, 'a');
    if(keySharedMem == -1)
        errExit("Ftok for shdmem failed!");
    const int lengthMemory = LENGTH_SHARED_MEM;
    int totalLength = sizeof(struct Memoryrow) * lengthMemory;
    printf("Total length of memory is %d: \n", totalLength);
    shmid = createSharedMemory(keySharedMem, totalLength);
    mempointer = (struct Memoryrow*) attachSharedMemory(shmid, 0);
    printf("This is the shared mem id: %d\n", shmid);

    //SEMAFORO: CREAZIONE E INIZIALIZZIONE
    key_t keySem = ftok(IPC_SEM_KEY_PATH, 'a');
    if(keySem == -1)
        errExit("Ftok for semaphore failed!");
    semid = createSemaphore(keySem, 1);			
    printf("This is the semaphore id: %d\n", semid);
    union semun sem;
    sem.val = 1;				//inzializza il semaforo a 1 poi applica con semctl (i semafori vanno sempre iniziallizati)
    semctl(semid, 0, SETVAL, sem);		//array[0] = sem.val = 1
    
    //Creazione processo figlio (keymanager)
    pid_t pid = fork();
    if(pid == 0) {
        //Codice del figlio

        //RESET SIGN*AL SIGTERM
        if (signal(SIGTERM, SIG_DFL) == SIG_ERR )
            errExit("Error resetting sigterm for child process");   
            
        //Rimozione di SIGALARM dalla maschera dei segnali    
        if(sigdelset(&signalset, SIGALRM) == -1)       
            errExit("Error removing alarm from mask set");
        if(sigprocmask(SIG_SETMASK, &signalset, NULL) == -1)
            errExit("Error setting mask for ALRM");
        
	//start keymanager
        keymanager(shmid, semid, mempointer);
    } else {						
        //Codice del padre(SERVER) //fork: ritorna 0 per il figlio e il pid del figlio nel padre
        childpid = pid;

	//FIFO SERVER
        if(mkfifo(serverFifoPath, S_IRUSR | S_IWUSR | S_IRGRP) == -1) {
            errExit("Error creation Server FIFO");
        }
        printf("<Server> Attesa di un client\n");

        serverFifoFD = open(serverFifoPath, O_RDONLY);
        if(serverFifoFD == -1)
            errExit("Reading server fifo Failed");
        
        //Apertura della extra fifo in write per l'EOF 
        serverFifoExtraFD = open(serverFifoPath, O_WRONLY);			//quando il client scrive la server fifo deve rimanerer aperta per prevenire l'EOF, If the write end of a pipe is closed, then a process reading from
//the pipe will see end-of-file once it has read all remaining data in the pipe.
        if(serverFifoExtraFD == -1)
            errExit("Writing server fifo Failed");
        
        //Gestione delle richieste del client e inserimento nella memoria condivisa
        int bufferRead = -1;
        struct Request request;
        do {										
            bufferRead = read(serverFifoFD, &request, sizeof(struct Request));
            if(bufferRead != sizeof(struct Request)) {
                printf("Request read failed, incompatible or size differents\n");
            } else {
                struct Response response;
                response.key = -1;							
                const int service = getService(request.service);
                if(service == NO_SERVICE)
                    printf("<Server> Richiesta ricevuta da client <%s PID: %d>, servizio richiesto inesistente \n", request.user_code, request.pid);
                else    
                    printf("<Server> Richiesta ricevuta da client <%s PID: %d>, servizio richiesto: %i \n", request.user_code, request.pid, service);
                if(service != NO_SERVICE) {
                    //Generazione della chiave e inserimento nella memoria condivisa
                    long key = generateKey(service, request.user_code);	
                    response.key = key;
                    
                    if(insertKey(key, request.user_code) == 0) {
                        printf("Memoria piena!! Chiave non inserita\n");
                        response.key = 0; 
                    }
                    requestNumber++;
                } else {
                    printf("Servizio richiesto non disponibile!\n");
                    response.key = -1;
                }
                
		//Apertura di FIFO CLIENT e invio della response
                char clientFifoPath[100];
                sprintf(clientFifoPath, "%s_%d", baseClientFifoPath, request.pid);
                int clientFifoPathFD = open(clientFifoPath, O_WRONLY);
                if(clientFifoPathFD == -1) {
                    printf("Client FIFO open error!\n");
                } else {
                    if(write(clientFifoPathFD, &response, sizeof(struct Response)) !=
                        sizeof(struct Response)) {
                        printf("Write to client FIFO has failed\n");
                    } else {
                        printf("<Server> Risposta inviata a client <%s PID: %d>, chiave generata %li\n", request.user_code, request.pid, response.key);
                    }
                }
                if(close(clientFifoPathFD) == -1)
                    printf("Error closing FIFO client\n");
            }
            printf("<Server> Attesa di un client\n");
        } while(bufferRead != -1);
    }

    //Uscita e chiusura di tutte le risorse(solo in caso di rottura della fifo)
    quit();
}
