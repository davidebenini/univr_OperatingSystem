#ifndef _REQUEST_RESPONSE_HH
#define _REQUEST_RESPONSE_HH

#include "constant.h"

struct Request {
    char user_code[USER_CODE_LENGTH];
    char service[USER_CODE_LENGTH];
    pid_t pid;
};

struct Response {
    long key;
};

#endif