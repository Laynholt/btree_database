#ifndef _ENTITY_H_
#define _ENTITY_H_

#include <stdint.h>

typedef struct Data
{
    uint32_t age;
    char name[20];
    char surname[20];

}Data;

typedef struct Entity
{
    uint64_t key;
    Data data;

}Entity;

#endif
