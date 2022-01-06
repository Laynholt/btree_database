#ifndef _CLIENT_H_
#define _CLIENT_H_

#include "../cli-ser-lib.h"

typedef struct Client
{
    uint32_t client_id;
    pthread_t threads;

}Client;

static Client clients;

#endif
