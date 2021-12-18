#include "../tree/tree.h"
#include "../hash/hash.h"

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
    
    while(i++ < 10)
    {
        //printf("Enter key: ");
        scanf("%s", str);
        entity.key = hash(str);
        //entity.key = hash((unsigned char*)keys[i]);
        printf("Your key is [%lu]\n", entity.key);

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

    uint64_t k;

    printf("Key to delete: ");
    scanf("%lu", &k);
    remove_entity(&tree, k);

    print_tree(&tree);
    
    printf("Key to delete: ");
    scanf("%lu", &k);
    remove_entity(&tree, k);

    print_tree(&tree);
    destroy_tree(&tree);

    return 0;
}
