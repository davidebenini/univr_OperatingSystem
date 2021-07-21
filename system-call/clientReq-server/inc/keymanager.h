#ifndef _KEYMANAGER_HH
#define _KEYMANAGER_HH
#include "../inc/sharedmemory.h"

int keymanager(int shmid, int semid, struct Memoryrow *mempointer);
int insertKey(long key, char userCode[]);
void deleteInvalidKeys();

#endif
