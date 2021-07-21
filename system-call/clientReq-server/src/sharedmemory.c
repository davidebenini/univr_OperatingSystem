#include <sys/shm.h>
#include <sys/stat.h>

#include "../inc/errExit.h"
#include "../inc/sharedmemory.h"

//Creazione della memoria condivisa
int createSharedMemory(key_t key, size_t size) {					
    int shmid = shmget(key, size, IPC_CREAT |  S_IWGRP | S_IRGRP | S_IRUSR | S_IWUSR);
    if(shmid == -1) {
        errExit("Creation memory failed");
    }
    return shmid;
}

int createSharedMemoryFromSystem(size_t size) {		
    int shmid = shmget(IPC_PRIVATE, size, IPC_CREAT | S_IWGRP | S_IRGRP | S_IRUSR | S_IWUSR); 	//
    if(shmid == -1) {
        errExit("Creation memory failed");
    }
    return shmid;
}

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

//remove della memoria condivisa
void removeSharedMemory(int shimd) {
    if(shmctl(shimd, IPC_RMID, NULL) == -1)
        errExit("Remove shared memory failed");
}
