#ifndef _SHR_MEMORY_H_
#define _SHR_MEMORY_H_

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

#include "../tree/tree.h"

static const uint64_t m = M;
static const uint64_t NODE_MAX_SIZE = sizeof(Node) + m * sizeof(Entity); 

extern int32_t create_shared_memory();
extern int16_t destroy_shared_memory(int32_t segment_id);

extern void* attach_shared_memory(int32_t segment_id);
extern int16_t dettach_shared_memory(void* shm_ptr);

extern int16_t filling_shared_memory(int32_t segment_id, void* shm_ptr, const  Node* node);
extern int16_t getting_from_shared_memory(int32_t segment_id, void* shm_ptr, Node* node);

#endif
