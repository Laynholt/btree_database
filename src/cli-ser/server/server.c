#include "server.h"

static int16_t read_data_from_file(char* str, Btree* tree)
{
    FILE* file = fopen(str, "r");

    if (file == NULL)
    {
        perror("Error opening file: ");
        return -1;
    }

    char buf[100];
    unsigned char key_str[30];
    Entity entity;

    while(fgets(buf, 100, file) != NULL)
    {
        sscanf(buf, "%s %s %s %u", key_str, entity.data.name, entity.data.surname, &entity.data.age);
        entity.key = hash(key_str);

        add_entity(tree, &entity);
    }

    fclose(file);
    return 1;
}

static void* client_wrapper(void* arg)
{
    char buffer[BUFFER_SIZE];
    char commands[8];

    uint8_t is_shm_ok = TRUE;
    int32_t ret_val;

    Client* cli = (Client*)arg;
    Shrm shrm_node, shrm_ent;

    printf("Client with id [%d] was connected!\n", cli->client_id);

    shrm_node.seg_id = create_shared_memory(SHRM_NODE_MAX_SIZE, cli->client_id);
    shrm_ent.seg_id = create_shared_memory(SHRM_ENTITY_MAX_SIZE, cli->client_id + 100);

    if (shrm_ent.seg_id == -1 || shrm_node.seg_id == -1)
    { 
        is_shm_ok = FALSE;
    }

    if (is_shm_ok)
    {
        shrm_node.shrm_ptr = attach_shared_memory(shrm_node.seg_id);
        shrm_ent.shrm_ptr = attach_shared_memory(shrm_ent.seg_id);
    }

    if (shrm_node.shrm_ptr == (void*)-1 || shrm_ent.shrm_ptr == (void*)-1)
    {
        is_shm_ok = FALSE;

        destroy_shared_memory(shrm_node.seg_id);
        destroy_shared_memory(shrm_ent.seg_id);
    }

    
    // shrm status
    ret_val = write(cli->client_id, &is_shm_ok, sizeof(uint8_t));
    if (ret_val == -1)
    {
        perror("Write1 error: ");
    }

    if (is_shm_ok)
    {

        // send shrm for node
        ret_val = write(cli->client_id, &shrm_node, sizeof(Shrm));
        if (ret_val == -1)
        {
            perror("Write2 error: ");
        }

        // send shrm for entity
        ret_val = write(cli->client_id, &shrm_ent, sizeof(Shrm));
        if (ret_val == -1)
        {
            perror("Write3 error: ");
        }

        // command mode
        while(TRUE)
        {
            buffer[0] = '\0';

            // client command
            ret_val = read(cli->client_id, buffer, BUFFER_SIZE);
            if (ret_val == -1)
            {
                perror("Read1 error: ");
            }

            buffer[BUFFER_SIZE - 1] = '\0';
            sscanf(buffer, "%s", commands);

            // if we get signal to close server
            if (is_closing == TRUE)
            {
                strcpy(buffer, "close");
                ret_val = write(cli->client_id, buffer, strlen(buffer) + 1);
                if (ret_val == -1)
                {
                    perror("Write4 error: ");
                }

                printf("Disconnect client [%d]..\n", cli->client_id);
                break;
            
            }

            // send answer to client
            ret_val = write(cli->client_id, buffer, strlen(buffer) + 1);
            if (ret_val == -1)
            {
                perror("Write5 error: ");
            }


            // check commands
            // print tree
            if (!strncmp(buffer, "print", BUFFER_SIZE))
            {
                pthread_mutex_lock(&mutex);
                
                char* path = "cli-ser/server/print.txt";
                print_tree_to_file(&tree, path);
               
                struct stat obj;
                stat(path, &obj);
                int32_t size = obj.st_size;

                // send for malloc
                ret_val = write(cli->client_id, &size, sizeof(int32_t));
                if (ret_val == -1)
                {
                    perror("Write6 error: ");
                }

                int32_t filehandle = open(path, O_RDONLY);
                sendfile(cli->client_id, filehandle, NULL, size);
                close(filehandle);
                remove(path);

                pthread_mutex_unlock(&mutex);
            }
            // get node -> push to shrm
            else if (!strncmp(buffer, "get", BUFFER_SIZE))
            {
                uint32_t node_id = 0;
                
                ret_val = read(cli->client_id, &node_id, sizeof(uint32_t));
                if (ret_val == -1)
                {
                    perror("Read3 error: ");
                }

                pthread_mutex_lock(&mutex);
            
                Node* node = NULL;
                node = find_node(tree.root, node_id);
                
                if (node != NULL)
                {
                    push_node_shared_memory(shrm_node.seg_id, shrm_node.shrm_ptr, node);
                    strcpy(buffer, "found");    
                }
                else
                {
                    strcpy(buffer, "not found");
                }

                ret_val = write(cli->client_id, buffer, strlen(buffer) + 1);
                if (ret_val == -1)
                {
                    perror("Write7 error: ");
                }

                pthread_mutex_unlock(&mutex);
            }
            // add entity to tree -> get this entity from shrm
            else if (!strncmp(buffer, "add", BUFFER_SIZE))
            {
                Entity ent;
                int32_t t;

                // client written entity
                ret_val = read(cli->client_id, &t, sizeof(int32_t));
                if (ret_val == -1)
                {
                    perror("Read2 error: ");
                }

                if (t == 1)
                {
                    pthread_mutex_lock(&mutex);
                    
                    pull_entity_shared_memory(shrm_ent.seg_id, shrm_ent.shrm_ptr, &ent);
                    add_entity(&tree, &ent);
                    
                    pthread_mutex_unlock(&mutex);
                }
            }
            // remove entity from tree -> get entity's key from shrm
            else if (!strncmp(buffer, "remove", BUFFER_SIZE))
            {
                char key_str[MAX_KEY_SIZE] = {'\0'};
                uint64_t key;
                
                ret_val = read(cli->client_id, key_str, MAX_KEY_SIZE);
                if (ret_val == -1)
                {
                    perror("Read3 error: ");
                }

                //key = hash(key_str);
                sscanf(key_str, "%lu", &key);

                pthread_mutex_lock(&mutex);
                remove_entity(&tree, key);
                pthread_mutex_unlock(&mutex);
            }
            // client disconnect
            else if (!strncmp(buffer, "exit", BUFFER_SIZE))
            {
                break;
            }
        }

        // closing client
            
        dettach_shared_memory(shrm_ent.shrm_ptr);
        dettach_shared_memory(shrm_node.shrm_ptr);

        destroy_shared_memory(shrm_ent.seg_id);
        destroy_shared_memory(shrm_node.seg_id);
    }

    close(cli->client_id);
    printf("Client with id [%d] was disconnected!\n", cli->client_id);
    cli->is_empty = TRUE;
    cli->client_id = 0;

    return (void*)1;
}

static void close_server()
{
    is_closing = TRUE;
    printf("Free all dynamic data...\n");
    for(uint32_t i = 0; i < clients.size; ++i)
    {
        if (clients.clients[i].is_empty == FALSE)
        {
            printf("Waiting while client [%d] disconnect.\n", clients.clients[i].client_id);
            pthread_join(clients.clients[i].threads, NULL);
        }
    }

    if (connection_socket != -1)
    {
        close(connection_socket);
    }
    
    unlink(SOCKET_NAME);
    destroy_tree(&tree);
}

void signal_int(int32_t sig)
{
    printf("\nClosing server...\n");
    close_server();
    exit(sig);
}

int main()
{
    // flag to close server;
    is_closing = FALSE;

    signal(SIGINT, signal_int);

    if(atexit(close_server))
        printf("Atexit error.\n");

    char str[] = "cli-ser/server/data.txt";

    // create tree
    create_tree(&tree);
    read_data_from_file(str, &tree);

    printf("Server was started!\n");

    // we dont have clients now
    clients.size = MAX_CLIENT_CONNECT;
    for(uint32_t i = 0; i < MAX_CLIENT_CONNECT; ++i)
    {
        clients.clients[i].is_empty = TRUE;
        clients.clients[i].threads = 0;
        clients.clients[i].client_id = 0;
    }

    // create socket
    struct sockaddr_un name;
    int16_t ret_val;
    int32_t data_socket;
       
    unlink(SOCKET_NAME);
    connection_socket = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if (connection_socket == -1)
    {
        perror("Socker error: ");
        close_server();
        return -1;
    }
    
    memset(&name, 0, sizeof(struct sockaddr_un));
    name.sun_family = AF_UNIX;
    strncpy(name.sun_path, SOCKET_NAME, sizeof(name.sun_path) - 1);
    
    ret_val = bind(connection_socket, (const struct sockaddr*) &name, sizeof(struct sockaddr_un));
    if (ret_val == -1)
    {
        perror("Bind error: ");
        close_server();
        return -1;
    }

    ret_val = listen(connection_socket, MAX_CLIENT_CONNECT);
    if (ret_val == -1)
    {
        perror("Listen error: ");
        close_server();
        return -1;
    }

    while(TRUE)
    {
        data_socket = accept(connection_socket, NULL, NULL);
        if (data_socket == -1)
        {
            perror("Accept error: ");
            continue;
        }
    
        pthread_mutex_lock(&mutex);    

        // if clients.size != 0
        uint8_t was_found = FALSE;
        uint32_t index;
        for(uint16_t i = 0; i < clients.size; ++i)
        {
            if (clients.clients[i].is_empty == TRUE)
            {
                index = i;
                was_found = TRUE;
                clients.clients[i].client_id = data_socket;
                clients.clients[i].is_empty = FALSE;
                break;
            }
        }

        if (was_found)
        {
            char conn[] = "You was connected to the server!\n";
            ret_val = write(data_socket, conn, 34);
            if (ret_val == -1)
            {
                perror("Write error: ");
            }
           pthread_create(&clients.clients[index].threads, NULL, client_wrapper, (void*)&clients.clients[index]);
        }

        else
        {
            char full[] = "Server is full. Try again later!\n";
            ret_val = write(data_socket, full, 34);
            if (ret_val == -1)
            {
                perror("Write error: ");
            }

            close(data_socket);
        }

        pthread_mutex_unlock(&mutex);
    }

    return 0;
}
