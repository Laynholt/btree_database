#ifndef _SERVER_H_
#define _SERVER_H_

#define SOCKET_NAME "my_socket_name"
#define BUFFER_SIZE 30
#define MAX_CLIENT_CONNECT 3
#define MAX_KEY_SIZE 40

#define TRUE 1
#define FALSE 0

#include "../tree/tree.h"
#include "../hash/hash.h"
#include "../shrm/shrm.h"

#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <fcntl.h>

static void print_entity(const Entity* entity);
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
static int32_t connection_socket;
static uint8_t is_closing;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

#endif
