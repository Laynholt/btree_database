#include "../tree/tree.h"
#include "../hash/hash.h"
#include "../shrm/shrm.h"

#include <stdio.h>

static void print_entity(Entity* entity)
{
    printf("Key [%lu]\nName [%s]\nSurname [%s]\nAge [%u]\n", entity->key, entity->data.name, entity->data.surname, entity->data.age);
}


int main()
{
    int16_t i = 0;
    unsigned char str[20];

    char* names[] = {"Josaf", "Laure", "Paul", "Kale", "John", "Hill", "Holly", "Gwen", "Peter", "Harly"};
    char* sur[] = {"Kofer", "Pauler", "Mackeyn", "Nihol", "Leaf", "Boray", "Shock", "Stacy", "Parker", "Quenn"};
    uint32_t ages[] = {23 , 25, 35, 34, 40, 53, 45, 28, 27, 22};
    char* keys[] = {"apple", "pine", "orange", "potato", "tomato", "cabage", "raspberry", "strawberry", "onion", "garlic"};

    Btree tree;
    create_tree(&tree);

    Entity entity;

    while(i++ < 30)
    {
        //printf("Enter key: ");
        scanf("%s", str);
        entity.key = hash(str);
        //entity.key = hash((unsigned char*)keys[i]);
        //printf("Your key is [%lu]\n", entity.key);

        //printf("Enter name: ");
        scanf("%s", entity.data.name);
        //memcpy(entity.data.name, names[i], strlen(names[i]) + 1);
        //printf("%s, %lu\n", entity.data.name, strlen(names[i]));
        //printf("Enter surname: ");
        scanf("%s", entity.data.surname);
        //memcpy(entity.data.surname, sur[i], strlen(sur[i]) + 1);
        //printf("%s, %lu\n", entity.data.surname, strlen(sur[i]));
        //printf("Enter age: ");
        scanf("%u", &entity.data.age);
        //entity.data.age = ages[i];
        
        add_entity(&tree, &entity);
    }

    print_tree(&tree);
/*
    uint64_t k;

    printf("Key to delete: ");
    scanf("%lu", &k);
    remove_entity(&tree, k);

    print_tree(&tree);
    
    printf("Key to delete: ");
    scanf("%lu", &k);
    remove_entity(&tree, k);

    print_tree(&tree);
*/
/*    Node my_node;
    printf("Node: %lu\n", sizeof(Node));
    printf("Entity: %lu\n", sizeof(Entity));
    printf("Node*: %lu\n", sizeof(Node*));
    printf("uint32_t: %lu\n", sizeof(uint32_t));
    
    printf("M: %d\n", M);
    
    uint64_t a = M;
    uint64_t b = sizeof(Node) + a * sizeof(Entity) + (a + 1) * sizeof(Node*) + 2 * sizeof(uint32_t);
    printf("Res: %lu\n", sizeof(Node) + a * sizeof(Entity) + (a + 1) * sizeof(Node*) + 2 * sizeof(uint32_t));
    printf("b: %lu\n", b);

    printf("%lu\n", NODE_MAX_SIZE);

    create_shared_memory();
*/  
    void* shm_ptr;
    int32_t seg_id = create_shared_memory();
    printf("\nID: %d\n", seg_id);

    shm_ptr = attach_shared_memory(seg_id);
    filling_shared_memory(seg_id, shm_ptr, tree.root);
    
    char c = '0';
    while(c != '1')
    {
        scanf("%c", &c);
    }

    dettach_shared_memory(shm_ptr);
    destroy_shared_memory(seg_id);


  /* 
    Node my_node;
    void* shm_ptr;
    int32_t seg_id = create_shared_memory();
  
    if (seg_id != -1)
    {
        shm_ptr = attach_shared_memory(seg_id);
        filling_shared_memory(seg_id, shm_ptr, tree.root);
        getting_from_shared_memory(seg_id, shm_ptr, &my_node);

//        my_node.entities[0].key = 666;
        for(uint16_t i = 0; i < my_node.size; ++i)
        {
            print_entity(&my_node.entities[i]);
        }
        free(my_node.entities);

        dettach_shared_memory(shm_ptr);
        destroy_shared_memory(seg_id);
    }
    */
    print_tree(&tree);

    destroy_tree(&tree);

    return 0;
}
