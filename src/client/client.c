#include "client.h"

static void print_entity(const Entity* entity)
{
    printf("Key - [%lu], Name - [%s], Surname - [%s], Age - [%u]\n", entity->key, entity->data.name, entity->data.surname, entity->data.age);
}

int main()
{
    struct sockaddr_un name;
    int32_t ret_val;
    int32_t  data_socket;
    char command_buffer[BUFFER_SIZE];
    char command[8];

    Node node;
    Entity entity;

    node.size = 0;

    data_socket = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if (data_socket == -1) 
    {
        perror("Socket error: ");
        return -1;
    }

    memset(&name, 0, sizeof(struct sockaddr_un));
    name.sun_family = AF_UNIX;
    strncpy(name.sun_path, SOCKET_NAME, sizeof(name.sun_path) - 1);
    
    ret_val = connect(data_socket, (const struct sockaddr*) &name, sizeof(struct sockaddr_un));
    if (ret_val == -1) 
    {
        fprintf(stderr, "Сервер выключен.\n");
        return -1;
    }

    Shrm shrm_node, shrm_ent;
    uint8_t is_shm_ok = TRUE;

    ret_val = read(data_socket, &is_shm_ok, sizeof(uint8_t));
    if (ret_val == -1)
    {
        perror("Write error: ");
    }

    if (is_shm_ok)
    {
        // accept shrm for node
        ret_val = read(data_socket, &shrm_node, sizeof(Shrm));
        if (ret_val == -1)
        {
            perror("Read error: ");
            is_shm_ok = FALSE;
        }

        // accept shrm for entity
        ret_val = read(data_socket, &shrm_ent, sizeof(Shrm));
        if (ret_val == -1)
        {
            perror("Read error: ");
            is_shm_ok = FALSE;
        }

        //printf("N: %d, E: %d\n", shrm_node.seg_id, shrm_ent.seg_id);

        if (!is_shm_ok)
        {
            strcpy(command_buffer, "exit");
            ret_val = write(data_socket, command_buffer, strlen(command_buffer) + 1);
            if (ret_val == -1)
            {
                perror("Write error: ");
            }
        }

        else
        {
            shrm_ent.shrm_ptr = attach_shared_memory(shrm_ent.seg_id);
            shrm_node.shrm_ptr = attach_shared_memory(shrm_node.seg_id);

            if (shrm_ent.shrm_ptr == (void*)-1 || shrm_node.shrm_ptr == (void*)-1)
            {
                perror("Cant attach-:");
                is_shm_ok = FALSE;
                
                strcpy(command_buffer, "exit");
                ret_val = write(data_socket, command_buffer, strlen(command_buffer) + 1);
                if (ret_val == -1)
                {
                    perror("Write error: ");
                }
            }

            while(TRUE && is_shm_ok)
            {
                printf("Enter command [print | get | add | remove | exit]: ");
                scanf("%s", command_buffer); 

                // send command
                ret_val = write(data_socket, command_buffer, strlen(command_buffer) + 1);
                if (ret_val == -1)
                {
                    perror("Write error: ");
                }

                // accept that command from server
                ret_val = read(data_socket, command_buffer, strlen(command_buffer) + 1);
                if (ret_val == -1)
                {
                    perror("Read error: ");
                }


                sscanf(command_buffer, "%s", command);
                // compare commands
                if (!strncmp(command, "print", 8))
                {
                    int32_t size;

                    // get file size
                    ret_val = read(data_socket, &size, sizeof(int32_t));
                    if (ret_val == -1)
                    {
                        perror("Read error: ");
                    }

                    size++;
                    char* str = (char*)malloc(size);
                    
                    // get file data
                    ret_val = read(data_socket, str, size);
                    if (ret_val == -1)
                    {
                        perror("Read error: ");
                    }
                    str[size-1] = '\0';
                    
                    int32_t filehandle = open("temp.txt", O_CREAT | O_WRONLY, 0666);
                    write(filehandle, str, size - 1);
                    close(filehandle);

                    printf("[Tree:]\n\n%s\n", str);

                    free(str);
                }   

                else if (!strncmp(command, "get", 8))
                {
                    uint32_t node_id = 0;
                    printf("Enter node id: ");
                    scanf("%u", &node_id);

                    ret_val = write(data_socket, &node_id, sizeof(uint32_t));
                    if (ret_val == -1)
                    {
                        perror("Read error: ");
                    }

                    // found or not
                    ret_val = read(data_socket, command_buffer, BUFFER_SIZE);
                    if (ret_val == -1)
                    {
                        perror("Read error: ");
                    }

                    if (!strncmp(command_buffer, "found", BUFFER_SIZE))
                    {
                        pull_node_shared_memory(shrm_node.seg_id, shrm_node.shrm_ptr, &node);
                        for(uint16_t i = 0; i < node.size; ++i)
                        {
                            print_entity(&node.entities[i]);
                        }
                    }

                    else
                    {
                        printf("Node with this id doesnt exists!\n");
                    }
                }

                else if (!strncmp(command, "add", 8))
                {
                    int32_t t = 1;
                    unsigned char key_str[30];

                    printf("Enter key: ");
                    scanf("%s", key_str);
                    entity.key = hash(key_str);
                    printf("Enter name: ");
                    scanf("%s", entity.data.name);
                    printf("Enter surname: ");
                    scanf("%s", entity.data.surname);
                    printf("Enter age: ");
                    scanf("%u", &entity.data.age);

                    push_entity_shared_memory(shrm_ent.seg_id, shrm_ent.shrm_ptr, &entity);
                    
                    // ack for server
                    ret_val = write(data_socket, &t, sizeof(int32_t));
                    if (ret_val == -1)
                    {
                        perror("Write error: ");
                    }
                
                }

                else if (!strncmp(command, "remove", 8))
                {
                    char key_str[MAX_KEY_SIZE] = { '\0' };
                    
                    printf("Enter key (For example: 6993835389): ");
                    scanf("%s", key_str);

                    ret_val = write(data_socket, key_str, MAX_KEY_SIZE);
                    if (ret_val == -1)
                    {
                        perror("Write error: ");
                    }
                }

                else if (!strncmp(command, "close", 8))
                {
                    printf("Server is closing. Disconnecting...\n");
                    break;
                }

                else if (!strncmp(command, "exit", 8))
                {
                    break;
                }
            }
          
            // clear all data 
            
            if (is_shm_ok)
            {
                dettach_shared_memory(shrm_node.shrm_ptr);
                dettach_shared_memory(shrm_ent.shrm_ptr);
            }

            if (node.size > 0)
            {
                free(node.entities);
            }
        }
    }

    close(data_socket);
    return 0;


    /*Node my_node;
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
   
    return 0;*/
}
