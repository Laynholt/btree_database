#include "cli-ser-lib.h"

void print_entity(const Entity* entity)
{
    printf("Key - [%lu], Name - [%s], Surname - [%s], Age - [%u]\n", entity->key, entity->data.name, entity->data.surname, entity->data.age);
}
