#ifndef _KEYGENERATOR_HH
#define _KEYGENERATOR_HH

struct Node {
    long value;
    struct Node *next;
};

long generateKey(int service, char *user_code);

int containsKey(long key);

int addNode(long key);

#endif
