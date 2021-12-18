/*
 *      Usefull info:
 *          > http://www.cse.yorku.ca/~oz/hash.html
 *
 *
 */

#include "hash.h"

uint64_t hash(unsigned char *str)
{
    uint64_t hash = 5381;
    uint16_t c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}
