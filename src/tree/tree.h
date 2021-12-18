#ifndef _TREE_H_
#define _TREE_H_

#include "entity.h"

#include <stdio.h>
#include <malloc.h>
#include <string.h>

#define T 3
#define M 2*T-1

typedef struct Node
{
    uint8_t size;
    uint8_t is_leaf;
    uint16_t id_node;
    struct Node* parent;
    struct Node** childs;
    Entity* entities;    
    
}Node;

typedef struct Btree
{
    Node* root;
    uint16_t tree_size;

}Btree;

extern void create_tree(Btree* tree);
extern void destroy_tree(Btree* tree);

extern void add_entity(Btree* tree, Entity* entity);
extern int16_t remove_entity(Btree* tree, uint64_t key);

extern Entity* find_entity(Btree* tree, uint64_t key);
extern void print_tree(Btree* tree);

#endif
