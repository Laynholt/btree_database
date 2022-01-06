#ifndef _CLI_SER_LIB_H_
#define _CLI_SER_LIB_H_

#define SOCKET_NAME "my_socket_name"
#define BUFFER_SIZE 30
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


extern void print_entity(const Entity* entity);

static int32_t connection_socket;

#endif
