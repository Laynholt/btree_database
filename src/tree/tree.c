#include "tree.h"

#define TRUE 1
#define FALSE 0

static uint16_t id = 0;
static uint8_t is_incorrected_tree_degree = 0;

static void destroy_node(Node* node);

static void split_root(Node* node, Btree* tree);
static void split_node(Node* node, Btree* tree);
static void check_fullness_node(Node* node, Btree* tree);

static void add_entity_to_node(Node* node, Entity* entity, Btree* tree);
static int16_t remove_entity_from_leaf_node(Node* node, uint64_t key, uint16_t child_id, Btree* tree);    
static int16_t remove_entity_from_middle_node(Node* node, uint64_t key, uint16_t child_id, Btree* tree);

static void print_node(Node* node, uint16_t offset);
static void print_node_to_file(Node* node, uint16_t offset, FILE* file);
static void repair_tree(Node* start_node, Btree* tree);


void create_tree(Btree* tree)
{
    tree->root = NULL;
    tree->tree_size = 0;

    if (T < 2)
    {
        printf("Incorrected tree degree! Must be >=2!\n");
        is_incorrected_tree_degree = 1;
    }
}

void destroy_tree(Btree* tree)
{
    if (tree->root == NULL)
    {
        return;
    }

    else if (tree->root->is_leaf)
    {
        free(tree->root->entities);
        free(tree->root);
        tree->root = NULL;
        tree->tree_size = 0;
    }

    else
    {
        for(uint16_t i = 0; i < tree->root->size + 1; ++i)
        {
            destroy_node(tree->root->childs[i]);
        }
        tree->root->is_leaf = TRUE;
        free(tree->root->childs);
        destroy_tree(tree);
    }
}

static void destroy_node(Node* node)
{
    if (node->is_leaf)
    {
        free(node->entities);
        free(node);
    }

    else
    {
        for(uint16_t i = 0; i < node->size + 1; ++i)
        {
            destroy_node(node->childs[i]);
        }
        free(node->childs);
        node->is_leaf = TRUE;
        destroy_node(node);
    }
}

void add_entity(Btree* tree, Entity* entity)
{
    if (is_incorrected_tree_degree)
        return;

    // Dont have element
    if (tree->root == NULL)
    {
        tree->root = (Node*)malloc(sizeof(Node));
        tree->tree_size = 1;
        tree->root->id_node = id++;
 
        tree->root->parent = NULL;
        tree->root->childs = NULL;
        tree->root->is_leaf = TRUE;
        tree->root->entities = (Entity*)malloc(sizeof(Entity));
        
        memcpy(tree->root->entities, entity, sizeof(Entity));
        tree->root->size = 1;
    }

    // If we have root element
    else
    {
        // root isnt full and doesnt have childs
        if (tree->root->is_leaf)
        {
            Entity* tmp_entity;
            tmp_entity = (Entity*)malloc(sizeof(Entity) * (tree->root->size + 1));

            uint8_t was_added = FALSE;
            for(uint16_t i = 0; i < tree->root->size; ++i)
            {
                if (entity->key == tree->root->entities[i].key)
                {
                    printf("Entity with that key already exists!\n");
                    free(tmp_entity);
                    return;
                }
                else if (entity->key < tree->root->entities[i].key)
                {
                    memcpy(tmp_entity + i, entity, sizeof(Entity));
                    memcpy(tmp_entity + (i + 1), &tree->root->entities[i], sizeof(Entity) * (tree->root->size - i));
                    was_added = TRUE;
                    break;
                }
                else
                {
                    memcpy(tmp_entity + i, &tree->root->entities[i], sizeof(Entity));
                }
            }

            if (!was_added)
            {
                memcpy(tmp_entity + tree->root->size, entity, sizeof(Entity));
            } 

            free(tree->root->entities);
            tree->root->entities = tmp_entity;
            tree->root->size++;

            check_fullness_node(tree->root, tree);
        }

        // if root has childs
        else if (!tree->root->is_leaf)
        {
            uint16_t index = 0;

            for(uint16_t i = 0; i < tree->root->size; ++i, ++index)
            {
                if (entity->key == tree->root->entities[i].key)
                {
                    printf("Entity with that key already exists!\n");
                    return;
                }

                else if (entity->key < tree->root->entities[i].key)
                {
                    add_entity_to_node(tree->root->childs[index], entity, tree);
                    check_fullness_node(tree->root->childs[index], tree);
                    return;                
                }
            }
            add_entity_to_node(tree->root->childs[tree->root->size], entity, tree);
            check_fullness_node(tree->root->childs[tree->root->size], tree);
        }
    }
}

static void split_root(Node* node, Btree* tree)
{
    uint16_t center = node->size / 2;
    Node* tmp_root, *child1, *child2;

    tmp_root = (Node*)malloc(sizeof(Node));
    child1 = (Node*)malloc(sizeof(Node));
    child2 = (Node*)malloc(sizeof(Node));

    Node** root_childs = (Node**)malloc(sizeof(Node*)*2);
    Node** child1_childs;
    Node** child2_childs;

    if (!node->is_leaf)
    {
        uint8_t odd = ((M + 1) & 1 ) == 1 ? 1 : 0;

        child1_childs = (Node**)malloc(sizeof(Node*) * ((M + 1) / 2 + odd));
        child2_childs = (Node**)malloc(sizeof(Node*) * ((M + 1) / 2));
    }

    // New root
    tmp_root->size = 1;
    tmp_root->id_node = node->id_node;
    tmp_root->is_leaf = FALSE;
    tmp_root->parent = NULL;
    tmp_root->childs = root_childs;
    tmp_root->entities = (Entity*)malloc(sizeof(Entity));
    memcpy(tmp_root->entities, &node->entities[center], sizeof(Entity));

    // root's childs
    uint8_t odd = (node->size & 1 ) == 1 ? 1 : 0;

    child1->size = center;
    child2->size = node->size - (center + 1);
    child1->id_node = id++;
    child2->id_node = id++;
    child1->parent = tmp_root;
    child2->parent = tmp_root;
    root_childs[0] = child1;
    root_childs[1] = child2;

    if (node->is_leaf)
    {
        child1->is_leaf = TRUE;
        child2->is_leaf = TRUE;
        child1->childs = NULL;
        child2->childs = NULL;
    }

    else
    {
        child1->is_leaf = FALSE;
        child2->is_leaf = FALSE;
        child1->childs = child1_childs;
        child2->childs = child2_childs;
        
        memcpy(child1_childs, node->childs, sizeof(Node*) * (child1->size + 1));
        memcpy(child2_childs, node->childs + child1->size + 1, sizeof(Node*) * (child2->size + 1));

        // change parent
        for(uint16_t i = 0; i < child1->size + 1; ++i)
        {
            child1_childs[i]->parent = child1;
        }

        for(uint16_t i = 0; i < child2->size + 1; ++i)
        {
            child2_childs[i]->parent = child2;
        }
    }

    child1->entities = (Entity*)malloc(sizeof(Entity) * child1->size);
    child2->entities = (Entity*)malloc(sizeof(Entity) * child2->size);

    memcpy(child1->entities, node->entities, sizeof(Entity) * child1->size);
    memcpy(child2->entities, node->entities + child1->size + 1, sizeof(Entity) * child2->size);

    tree->tree_size += 2;

    free(tree->root->entities);
    if (!tree->root->is_leaf)
        free(tree->root->childs);
    free(tree->root);

    tree->root = tmp_root;

    //print_tree(tree);
}

static void add_entity_to_node(Node* node, Entity* entity, Btree* tree)
{
    if (node->is_leaf)
    {
        Entity* tmp_entity;
        tmp_entity = (Entity*)malloc(sizeof(Entity) * (node->size + 1));

        uint8_t was_added = FALSE;
        for(uint16_t i = 0; i < node->size; ++i)
        {
            if (entity->key == node->entities[i].key)
            {
                printf("Entity with that key already exists!\n");
                free(tmp_entity);
                return;
            }
            else if (entity->key < node->entities[i].key)
            {
                memcpy(tmp_entity + i, entity, sizeof(Entity));
                memcpy(tmp_entity + (i + 1), &node->entities[i], sizeof(Entity) * (node->size - i));
                was_added = TRUE;
                break;
            }
            else
            {
                memcpy(tmp_entity + i, &node->entities[i], sizeof(Entity));
            }
        }

        if (!was_added)
        {
            memcpy(tmp_entity + node->size, entity, sizeof(Entity));
        } 

        free(node->entities);
        node->entities = tmp_entity;
        node->size++;
    } 

    else if (!node->is_leaf)
    {
        uint16_t index = 0;

        for(uint16_t i = 0; i < node->size; ++i, ++index)
        {
            if (entity->key == node->entities[i].key)
            {
                printf("Entity with that key already exists!\n");
                return;
            }

            else if (entity->key < node->entities[i].key)
            {
                add_entity_to_node(node->childs[index], entity, tree);
                check_fullness_node(node, tree);
                return;                
            }
        }
        add_entity_to_node(node->childs[node->size], entity, tree);
    }

    check_fullness_node(node, tree);
}

static void check_fullness_node(Node* node, Btree* tree)
{
    // split all full nodes
    if (node->size == M)
    {
        Node* parent = node->parent;

        if (parent == NULL)
        {   
            split_root(node, tree);
        }
        else
        {
            split_node(node, tree);
            check_fullness_node(parent, tree);
        }
    }
}

static void split_node(Node* node, Btree* tree)
{
    uint16_t center = node->size / 2;
    Node* first_half_node, *second_half_node;

    first_half_node = (Node*)malloc(sizeof(Node));
    second_half_node = (Node*)malloc(sizeof(Node));

    Node** node_childs1;
    Node** node_childs2;

    if (!node->is_leaf)
    {
        uint8_t odd = ((M + 1) & 1 ) == 1 ? 1 : 0;

        node_childs1 = (Node**)malloc(sizeof(Node*) * ((M + 1) / 2 + odd));
        node_childs2 = (Node**)malloc(sizeof(Node*) * ((M + 1) / 2));
    }

    uint8_t was_added = FALSE;
    Entity* parent_entities = (Entity*)malloc(sizeof(Entity) * (node->parent->size + 1));

    // Reform parent
    // Adding entity
    for(uint16_t i = 0; i < node->parent->size; ++i)
    {
        if (node->entities[center].key < node->parent->entities[i].key)
        {

            memcpy(parent_entities + i, &node->entities[center], sizeof(Entity));
            memcpy(parent_entities + (i + 1), &node->parent->entities[i], sizeof(Entity) * (node->parent->size - i));
            was_added = TRUE;
            break;
        }
        else
        {
            memcpy(parent_entities + i, &node->parent->entities[i], sizeof(Entity));
        }
    }
    if (!was_added)
    {
        memcpy(parent_entities + node->parent->size, &node->entities[center], sizeof(Entity));
    }

    // Adding childs
    Node** parent_childs = (Node**)malloc(sizeof(Node*) * (node->parent->size + 2));   
    
    for(uint16_t i = 0; i < node->parent->size + 1; ++i)
    {
        if (node == node->parent->childs[i])
        {
            memcpy(parent_childs + i, &first_half_node, sizeof(Node*));
            memcpy(parent_childs + (i + 1), &second_half_node, sizeof(Node*));
            memcpy(parent_childs + (i + 2), &node->parent->childs[i + 1], sizeof(Node*) * (node->parent->size - i));
            was_added = TRUE;
            break;
        }
        else
        {
            memcpy(parent_childs + i, &node->parent->childs[i], sizeof(Node*));
        }
    }

   
    free(node->parent->childs);
    free(node->parent->entities);

    node->parent->childs = parent_childs;
    node->parent->entities = parent_entities;
    node->parent->size++;
   

    // Reform node
    first_half_node->size = center;
    second_half_node->size = node->size - (center + 1);
    first_half_node->id_node = node->id_node;
    second_half_node->id_node = id++;
    first_half_node->parent = node->parent;
    second_half_node->parent = node->parent;
   
    if (node->is_leaf)
    {
        first_half_node->is_leaf = TRUE;
        second_half_node->is_leaf = TRUE;
        first_half_node->childs = NULL;
        second_half_node->childs = NULL;
    }

    else
    {
        first_half_node->is_leaf = FALSE;
        second_half_node->is_leaf = FALSE;
        first_half_node->childs = node_childs1;
        second_half_node->childs = node_childs2;
        
        memcpy(node_childs1, node->childs, sizeof(Node*) * (first_half_node->size + 1));
        memcpy(node_childs2, node->childs + first_half_node->size + 1, sizeof(Node*) * (second_half_node->size + 1));

        for(uint16_t i = 0; i < first_half_node->size + 1; ++i)
        {
            node_childs1[i]->parent = first_half_node;
        }

        for(uint16_t i = 0; i < second_half_node->size + 1; ++i)
        {
            node_childs2[i]->parent = second_half_node;
        }
    }

    // reform entities
    first_half_node->entities = (Entity*)malloc(sizeof(Entity) * first_half_node->size);
    second_half_node->entities = (Entity*)malloc(sizeof(Entity) * second_half_node->size);

    memcpy(first_half_node->entities, node->entities, sizeof(Entity) * first_half_node->size);
    memcpy(second_half_node->entities, node->entities + first_half_node->size + 1, sizeof(Entity) * second_half_node->size);

    tree->tree_size += 1;

    free(node->entities);
    if (node->is_leaf)
        free(node->childs);
    free(node);
}

void print_tree(Btree* tree)
{
    if (tree->root) 
    {
        print_node(tree->root, 0);
    }
    else
    {
        printf("Tree is empty.\n");
        return;
    } 

}

static void print_node(Node* node, uint16_t offset)
{
    for(uint16_t i = 0; i < node->size; ++i)
    {
        for(uint16_t j = 0; j < offset; ++j)
            printf("\t");
        printf("Node id [%u], key [%lu], user name [%s]\n", node->id_node, node->entities[i].key, node->entities[i].data.name);
    }
    printf("-------------------------------------\n");

    if (!node->is_leaf)
    {
        for(uint16_t i = 0; i < node->size + 1; ++i)
        {
            print_node(node->childs[i], offset + 1);
        }
    }   
}

void print_tree_to_file(Btree* tree, char* path)
{
    FILE* file = fopen(path, "w");
    
    if (file == NULL)
    {
        perror("Cant creare file: ");
        return;
    }

    if (tree->root) 
    {
        print_node_to_file(tree->root, 0, file);
    }
    else
    {
        fprintf(file, "Tree is empty.\n");
        fclose(file);
        return;
    }

    fclose(file);
}

static void print_node_to_file(Node* node, uint16_t offset, FILE* file)
{
    for(uint16_t i = 0; i < node->size; ++i)
    {
        for(uint16_t j = 0; j < offset; ++j)
            fprintf(file, "\t");
        fprintf(file, "Node id [%u], key [%lu], user name [%s]\n", node->id_node, node->entities[i].key, node->entities[i].data.name);
    }
    fprintf(file, "-------------------------------------\n");

    if (!node->is_leaf)
    {
        for(uint16_t i = 0; i < node->size + 1; ++i)
        {
            print_node_to_file(node->childs[i], offset + 1, file);
        }
    }   
}

int16_t remove_entity(Btree* tree, uint64_t key)
{
   if (tree->root->size == 0)
   {
       printf("Tree is empty!\n");
       return -1;
   }

   // if root is leaf
   else if (tree->root->is_leaf)
   {
       int16_t ret_value = remove_entity_from_leaf_node(tree->root, key, 0, tree);
       
       if (ret_value == 1)
       {
           printf("Entity with key [%lu] was removed.\n", key);
       }
       else
       {
           printf("Tree doesn't have entity with key [%lu].\n", key);
       }

       if (tree->root->size == 0)
       {
           free(tree->root);
           tree->root = NULL;
           tree->tree_size = 0;
       }

       return ret_value;
   }

   else if (!tree->root->is_leaf) 
   {
       int16_t ret_value = remove_entity_from_middle_node(tree->root, key, 0, tree);

       if (ret_value == 1)
       {
           printf("Entity with key [%lu] was removed.\n", key);
       }
       else
       {
           printf("Tree doesn't have entity with key [%lu].\n", key);
       }
       return ret_value;
   }
}

static int16_t remove_entity_from_leaf_node(Node* node, uint64_t key, uint16_t child_id, Btree* tree)
{

    if ((node->is_leaf && node->size > T - 1) || (node == tree->root))
    {
        Entity* tmp_entity;
        tmp_entity = (Entity*)malloc(sizeof(Entity) * (node->size - 1));

        uint8_t was_removed = FALSE;
        uint8_t in_node = FALSE;
        uint16_t index = 0;
       
        for(uint16_t i = 0; i < node->size; ++i)
        {
            if (key == node->entities[i].key)
            {
                in_node = TRUE;
                index = i;
                break;
            }
        }

        if (in_node)
        {
            memcpy(tmp_entity, node->entities, sizeof(Entity) * index);
            memcpy(tmp_entity + index, &node->entities[index + 1], sizeof(Entity) * (node->size - index - 1));
            was_removed = TRUE;
            node->size--;
        }

        if (was_removed)
        {
            free(node->entities);
            node->entities = tmp_entity;
            return 1;
        }
        else
        {
            free(tmp_entity); 
            return -1;
        }
    }

    else if (node->is_leaf && node->size == T - 1)
    {
        uint8_t in_node = FALSE;

        // try to find element in node
        for(uint16_t i = 0; i < node->size; ++i)
        {
            if (node->entities[i].key == key)
            {
                in_node = TRUE;
                break;
            }
        }

        if (!in_node)
            return -1;

        uint16_t brother_size, parent_entity_id;
        Node* brother; 

        if (child_id > 0)
        {
            brother_size = node->parent->childs[child_id - 1]->size;
            brother = node->parent->childs[child_id - 1];
            parent_entity_id = child_id - 1;
        }
        else if (child_id == 0)
        {
            brother_size = node->parent->childs[child_id + 1]->size;
            brother = node->parent->childs[child_id + 1];
            parent_entity_id = child_id;
        }

        // take brother's entities
        if (brother_size > T - 1)
        {
            Entity* node_entity, *brother_entity;
            uint16_t given_size = brother_size - (T - 1);

            given_size = given_size > 1 ? (given_size / 2) : given_size;

            node_entity = (Entity*)malloc(sizeof(Entity) * (node->size + given_size));
            brother_entity = (Entity*)malloc(sizeof(Entity) * (brother_size - given_size));

            // left node with right brother
            if (child_id == 0)
            {
                // fill node
                memcpy(node_entity, node->entities, sizeof(Entity) * node->size);
                memcpy(node_entity + node->size, &node->parent->entities[parent_entity_id], sizeof(Entity));
                memcpy(node_entity + node->size + 1, brother->entities, sizeof(Entity) * (given_size - 1));
            
                // rewrite parent entity
                memcpy(&node->parent->entities[parent_entity_id], &brother->entities[given_size - 1], sizeof(Entity));

                // fill brother
                memcpy(brother_entity, &brother->entities[given_size], sizeof(Entity) * (brother_size - given_size));
            }
            // right node with left brother
            else if (child_id > 0)
            {
                // fill node
                memcpy(node_entity, brother->entities + brother_size - (given_size - 1), sizeof(Entity) * (given_size - 1));
                memcpy(node_entity + given_size - 1, &node->parent->entities[parent_entity_id], sizeof(Entity));
                memcpy(node_entity + given_size, node->entities, sizeof(Entity) * node->size);
            
                // rewrite parent entity
                memcpy(&node->parent->entities[parent_entity_id], &brother->entities[brother_size - given_size], sizeof(Entity));

                // fill brother
                memcpy(brother_entity, brother->entities, sizeof(Entity) * (brother_size - given_size));
            }

                    
            free(node->entities);
            free(brother->entities);

            node->entities = node_entity;
            brother->entities = brother_entity;

            node->size = node->size + given_size;
            brother->size = brother_size - given_size;
            
            return remove_entity_from_leaf_node(node, key, child_id, tree);
        }

        // merge with brother
        else
        {
            Entity* node_entity, *parent_entity;
            Node** parent_childs;

            Node* parent = node->parent;

            node_entity = (Entity*)malloc(sizeof(Entity) * (2 * (T - 1) + 1));
            parent_entity = (Entity*)malloc(sizeof(Entity) * (parent->size - 1));
            parent_childs = (Node**)malloc(sizeof(Node*) * (parent->size));

            // fill new node
            if (child_id == 0)
            {
                memcpy(node_entity, node->entities, sizeof(Entity) * node->size);
                memcpy(node_entity + node->size, &parent->entities[parent_entity_id], sizeof(Entity));
                memcpy(node_entity + node->size + 1, brother->entities, sizeof(Entity) * brother->size);
            }
            else if (child_id > 0)
            {
                memcpy(node_entity, brother->entities, sizeof(Entity) * brother->size);
                memcpy(node_entity + brother->size, &parent->entities[parent_entity_id], sizeof(Entity));
                memcpy(node_entity + brother->size + 1, node->entities, sizeof(Entity) * node->size);
            }

            // rewrite parent data
            for(uint16_t i = 0, count = 0; i < parent->size; ++i)
            {
                if (i == parent_entity_id)
                    continue;
                memcpy(parent_entity + count++, &parent->entities[i], sizeof(Entity));
            }
            // because now we have 2 childs ptr on parent array, and we need delete them and write new ptr
            for(uint16_t i = 0, count = 0; i < parent->size + 1; ++i, ++count)
            {
                if (i == parent_entity_id)
                {
                    ++i;
                    memcpy(parent_childs + count, &node, sizeof(Node*));
                }
                else
                {
                    memcpy(parent_childs + count, &parent->childs[i], sizeof(Node*));
                }
            }

            node->size = 2 * (T - 1) + 1;
            free(node->entities);
            node->entities = node_entity;

            free(brother->entities);
            free(brother);

            parent->size--;
            free(parent->entities);
            free(parent->childs);
            parent->entities = parent_entity;
            parent->childs = parent_childs;

            tree->tree_size--;
            
            if (child_id > 0)
                --child_id;

            // after removing we need rebalanced and repair our tree
            int16_t ret_value;
            ret_value = remove_entity_from_leaf_node(node, key, child_id, tree);

            repair_tree(node, tree);
            return ret_value;
        }
    }

    else if (!node->is_leaf)
    {
        return remove_entity_from_middle_node(node, key, child_id, tree);
    }
}

static void repair_tree(Node* start_node, Btree* tree)
{
    Node* parent = start_node->parent;

    // rebalancing completed
    if (parent == NULL)
        return;

    if (parent->size < T - 1)
    {
        Entity* new_parent_entity, *new_grandparent_entity;
        Node** new_parent_childs, **new_grandparent_childs;

        Node* uncle, *grand;
        grand = parent->parent;

        if (grand == NULL)
            return;

        uint16_t parent_id, grand_entity_id;

        // find parent index
        for(uint16_t i = 0; i < grand->size + 1; ++i)
        {
            if (grand->childs[i] == parent)
            {
                parent_id = i;
                break;
            }
        }
        
        // we need left 
        if (parent_id == grand->size)
        {
            uncle = grand->childs[parent_id - 1];
            grand_entity_id = parent_id - 1;
        }
        else if (parent_id < grand->size)
        {
            uncle = grand->childs[parent_id + 1];
            grand_entity_id = parent_id;
        }


        new_parent_entity = (Entity*)malloc(sizeof(Entity) * (parent->size + uncle->size + 1));
        new_grandparent_entity = (Entity*)malloc(sizeof(Entity) * (grand->size - 1));
        new_parent_childs = (Node**)malloc(sizeof(Node*) * (parent->size + 1 + uncle->size + 1));
        new_grandparent_childs = (Node**)malloc(sizeof(Node*) * (grand->size));

        // fill new node
        if (parent_id == grand->size)
        {
            // rewrite parent entity
            memcpy(new_parent_entity, uncle->entities, sizeof(Entity) * uncle->size);
            memcpy(new_parent_entity + uncle->size, &grand->entities[grand_entity_id], sizeof(Entity));
            memcpy(new_parent_entity + uncle->size + 1, parent->entities, sizeof(Entity) * parent->size);

            // rewrite parent childs
            memcpy(new_parent_childs, uncle->childs, sizeof(Node*) * (uncle->size + 1));
            memcpy(new_parent_childs + uncle->size + 1, parent->childs, sizeof(Node*) * (parent->size + 1));
        }
        else if (parent_id < grand->size)
        {
            // rewrite parent entity
            memcpy(new_parent_entity, parent->entities, sizeof(Entity) * parent->size);
            memcpy(new_parent_entity + parent->size, &grand->entities[grand_entity_id], sizeof(Entity));
            memcpy(new_parent_entity + parent->size + 1, uncle->entities, sizeof(Entity) * uncle->size);

            // rewrite parent childs
            memcpy(new_parent_childs, parent->childs, sizeof(Node*) * (parent->size + 1));
            memcpy(new_parent_childs + parent->size + 1, uncle->childs, sizeof(Node*) * (uncle->size + 1));
        }
 
        // rewrite grand entity
        for(uint16_t i = 0, count = 0; i < grand->size; ++i)
        {
            if (grand_entity_id == i)
                continue;
            memcpy(new_grandparent_entity + count++, &grand->entities[i], sizeof(Entity));
        }

        // rewrite grand childs
        for(uint16_t i = 0, count = 0; i < grand->size + 1; ++i, ++count)
        {
            if (i == grand_entity_id)
            {
                ++i;
                memcpy(new_grandparent_childs + count, &parent, sizeof(Node*));
            }
            else
            {
                memcpy(new_grandparent_childs + count, &grand->childs[i], sizeof(Node*));
            }
        }        
        
        parent->size = parent->size + uncle->size + 1;
        free(parent->entities);
        free(parent->childs);

        parent->entities = new_parent_entity;
        parent->childs = new_parent_childs;

        grand->size--;
        free(grand->entities);
        free(grand->childs);

        grand->entities = new_grandparent_entity;
        grand->childs = new_grandparent_childs;
        
        // delete uncle
        free(uncle->entities);
        free(uncle->childs);
        free(uncle);

        tree->tree_size--;

        repair_tree(parent, tree);
    }
}

static int16_t remove_entity_from_middle_node(Node* node, uint64_t key, uint16_t child_id, Btree* tree)
{
    if(node->is_leaf)
    {
        return remove_entity_from_leaf_node(node, key, child_id, tree);
    }

    else
    {
        uint16_t entity_index = 0;
        uint8_t was_found = FALSE;
        
        for(uint16_t i = 0; i < node->size; ++i)
        {
            // find in node
            if (node->entities[i].key == key)
            {
                was_found = TRUE;
                entity_index = i;
                break;
            }

            // find in left childs
            else if (key < node->entities[i].key)
            {
                return remove_entity_from_middle_node(node->childs[i], key, i, tree);
            }
        }

        if(!was_found)
        {
            return remove_entity_from_middle_node(node->childs[node->size], key, node->size, tree);
        }
        Node* right_child = node->childs[entity_index + 1];

        // rewrite node entity with child entity
        memcpy(&node->entities[entity_index], &right_child->entities[0], sizeof(Entity));
        right_child->entities[0].key = 0;
        key = 0;

        // recursively removing
        return remove_entity_from_middle_node(right_child, key, entity_index + 1, tree);
    }
}

Node* find_node(Node* node, uint16_t node_id)
{
    if (node->id_node == node_id)
        return node;

    if (!node->is_leaf)
    {
        Node* _node = NULL;
        for(uint16_t i = 0; i < node->size + 1; ++i)
        {
            _node = find_node(node->childs[i], node_id);
            if (_node != NULL)
            {
                return _node;
            }
        }
    }
    return NULL;
}
