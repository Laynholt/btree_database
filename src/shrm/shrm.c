#include "shrm.h"
#include <assert.h>

int32_t create_shared_memory(uint64_t shm_size, uint32_t client_id)
{
    int32_t segment_id;
    
    key_t key = ftok("shrm/shrm.h", client_id);
    if(key == -1)
    {
        printf("Cant create key!\n");
        perror(NULL);
        return -1;
    }

    segment_id = shmget(key, shm_size, IPC_CREAT | 0666);
    if (segment_id == -1)
    {
        printf("Cant create shared memory!\n");
        perror(NULL);
        return -1;
    }
    return segment_id;
}

int16_t destroy_shared_memory(int32_t segment_id)
{
    int16_t ret_val = shmctl(segment_id, IPC_RMID, NULL);
    
    if (ret_val == -1)
    {
        printf("Cant destroy shared memory!\n");
    } 
    return ret_val;
}

void* attach_shared_memory(int32_t segment_id)
{
    void* shm_ptr = shmat(segment_id, NULL, 0);

    if ((void*)-1 == shm_ptr)
    {
        printf("Cant attach to shared memory!\n");
        perror(NULL);
        return (void*)-1;
    }
    return shm_ptr;
}

int16_t dettach_shared_memory(void* shm_ptr)
{
    int16_t ret_val = shmdt(shm_ptr);

    if (ret_val == -1)
    {
        printf("Cant dettach from shared memory!\n");
    }
    return ret_val;
}

int16_t push_node_shared_memory(int32_t segment_id, void* shm_ptr, const Node* node)
{
    size_t node_size = sizeof(Node);
    size_t uint_size = sizeof(uint32_t);
    size_t entity_size = sizeof(Entity);

    uint32_t entity_num = node->size;

    // copy to shared memory
    memcpy(shm_ptr, node, node_size);
    memcpy(shm_ptr + node_size, &entity_num, uint_size);
    memcpy(shm_ptr + node_size + uint_size, node->entities, entity_size * entity_num);

    return 1;
}

int16_t pull_node_shared_memory(int32_t segment_id, void* shm_ptr, Node* node)
{
    size_t node_size = sizeof(Node);
    size_t uint_size = sizeof(uint32_t);
    size_t entity_size = sizeof(Entity);

    uint32_t entity_num;

    // getting from shared memory
    memcpy(node, shm_ptr, node_size);
    memcpy(&entity_num, shm_ptr + node_size, uint_size);
    assert((node->entities = (Entity*)malloc(entity_size * entity_num)) && "Cant malloc memory for entities!\n");
    memcpy(node->entities, shm_ptr + node_size + uint_size, entity_size * entity_num);

    return 1;
}

int16_t push_entity_shared_memory(int32_t segment_id, void* shm_ptr, const Entity* entity)
{
    memcpy(shm_ptr, entity, sizeof(Entity));
    return 1;
}

int16_t pull_entity_shared_memory(int32_t segment_id, void* shm_ptr, Entity* entity)
{
    memcpy(entity, shm_ptr, sizeof(Entity));
    return 1;
}
