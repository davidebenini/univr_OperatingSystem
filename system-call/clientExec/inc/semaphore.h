#ifndef _SEMAPHORE_HH
#define _SEMAPHORE_HH

#include <stdlib.h>
#include <sys/sem.h>

//definizione della union per il semaforo
union semun {
    int val;
    struct semid_ds * buf;
    unsigned short * array;
};

void semOp (int semid, unsigned short semaphoreNumb, short operation);
int getSemaphore(key_t key, int numsem);

void enterInCriticalSection(int semId);
void exitFromCriticalSection(int semId);

#endif
