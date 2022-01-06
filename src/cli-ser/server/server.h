#ifndef _SERVER_H_
#define _SERVER_H_

#include "../cli-ser-lib.h"

#define MAX_CLIENT_CONNECT 3

static int16_t read_data_from_file(char* str, Btree* tree);
static void* client_wrapper(void* arg);

typedef struct Client
{
    int32_t client_id;
    pthread_t threads;
    uint8_t is_empty;

}Client;

typedef struct Client_info
{
    Client clients[MAX_CLIENT_CONNECT];
    uint32_t size;

}Client_info;

static Btree tree;
static Client_info clients;
static uint8_t is_closing;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

#endif
