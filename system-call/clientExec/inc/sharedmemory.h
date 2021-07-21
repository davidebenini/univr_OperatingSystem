#ifndef _SHAREDMEMORY_HH
#define _SHAREDMEMORY_HH

#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include "constant.h"

struct Memoryrow {
    char userCode[USER_CODE_LENGTH];
    long key;
    time_t timestamp;
};

//Creazione della memoria condivisa
int createSharedMemory(key_t key, size_t size);

//Ottieniamo la memoria condivisa in caso sià già presente, return -1
int getSharedMemory(key_t key, size_t size);

//attach della memoria condivisa
void *attachSharedMemory(int shmid, int shmflg);

//detach della memoria condivisa
void freeSharedMemory(void *ptrSharedMemory);

int findAndMark(char *usercode, long key, struct Memoryrow *ptrSharedMemory);

#endif
