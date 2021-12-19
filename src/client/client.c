#include "../tree/tree.h"
#include "../shrm/shrm.h"

static void print_entity(Entity* entity)
{
    printf("Key [%lu]\nName [%s]\nSurname [%s]\nAge [%u]\n", entity->key, entity->data.name, entity->data.surname, entity->data.age);
}

int main()
{
    Node my_node;
    void* shm_ptr;
    int32_t seg_id = create_shared_memory();
 
    shm_ptr = attach_shared_memory(seg_id);
    getting_from_shared_memory(seg_id, shm_ptr, &my_node);

    for(uint16_t i = 0; i < my_node.size; ++i)
    {
        print_entity(&my_node.entities[i]);
    }
    free(my_node.entities);

    dettach_shared_memory(shm_ptr);
   
    return 0;
}
