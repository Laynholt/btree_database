#ifndef _SHR_MEMORY_H_
#define _SHR_MEMORY_H_

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

#include "../tree/tree.h"

typedef struct Shrm
{
    int32_t seg_id;
    void* shrm_ptr;
    
}Shrm;


static const uint64_t m = M;
static const uint64_t SHRM_NODE_MAX_SIZE = sizeof(Node) + m * sizeof(Entity) + 2 * sizeof(uint32_t); 
static const uint64_t SHRM_ENTITY_MAX_SIZE = sizeof(Entity);

extern int32_t create_shared_memory(uint64_t shm_size, uint32_t client_id);
extern int16_t destroy_shared_memory(int32_t segment_id);

extern void* attach_shared_memory(int32_t segment_id);
extern int16_t dettach_shared_memory(void* shm_ptr);

extern int16_t push_node_shared_memory(int32_t segment_id, void* shm_ptr, const Node* node);
extern int16_t pull_node_shared_memory(int32_t segment_id, void* shm_ptr, Node* node);

extern int16_t push_entity_shared_memory(int32_t segment_id, void* shm_ptr, const Entity* entity);
extern int16_t pull_entity_shared_memory(int32_t segment_id, void* shm_ptr, Entity* entity);


#endif
