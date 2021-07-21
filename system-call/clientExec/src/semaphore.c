#include <sys/sem.h>
#include <stdio.h>

#include <sys/stat.h>


#include "../inc/semaphore.h"
#include "../inc/errExit.h"

void semOp (int semid, unsigned short semaphoreNumb, short operation) {
    struct sembuf sop = {.sem_num = semaphoreNumb, .sem_op = operation, .sem_flg = 0};

    if (semop(semid, &sop, 1) == -1)      
        errExit("semop failed");
}

int getSemaphore(key_t key, int numsem) {
    int semid = semget(key, numsem, S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP);
    if(semid == -1) {
        errExit("Creation Semaphore failed");
    }
    return semid;
}

void enterInCriticalSection(int semId) {
    semOp(semId, 0, -1);
}

void exitFromCriticalSection(int semId){
    semOp(semId, 0, 1);
}
