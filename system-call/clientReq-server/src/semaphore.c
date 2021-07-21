#include <sys/sem.h>
#include <stdio.h>

#include <sys/stat.h>

#include "../inc/semaphore.h"
#include "../inc/errExit.h"

void semOp (int semid, unsigned short semaphoreNumb, short operation) {
    struct sembuf sop = {.sem_num = semaphoreNumb, .sem_op = operation, .sem_flg = 0};

    if (semop(semid, &sop, 1) == -1){
        printf("semop failed, operation: %d\n", operation);
        errExit("semop failed");
    }
}

int createSemaphore(key_t key, int numsem) {
    int semid = semget(key, numsem, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP);
    if(semid == -1) {
        errExit("Creation Semaphore failed");
    }
    return semid;
}

int removeSemaphore(int semid) {
    if(semctl(semid, 0, IPC_RMID, 0) == -1) {
        errExit("Remove semaphore failed");
    }
    return 1;
}

void enterInCriticalSection(int semId) {
    semOp(semId, 0, -1);			
}

void exitFromCriticalSection(int semId) {
    semOp(semId, 0, 1);				
}
