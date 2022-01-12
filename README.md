# In-memory Database
This program implements a In-memory database based on B-Trees.
There is also a client-server implementation on via local sockets. Data is stored and transmitted via shared memory.

# Compile
make

# Info about project
The project consists of several parts:
  1) Library for working with B-trees
  2) Library for working with shared memory
  3) Client and server implementation

Structure B-tree:
  typedef struct Btree
  {
  Node* root;
  uint16_t tree_size;
  }Btree;
  
The tree itself consists of nodes that contain database elements, as well as pointers to child nodes.
  typedef struct Node
  {
  uint8_t size;
  uint8_t is_leaf;
  uint16_t id_node;
  struct Node* parent;
  struct Node** childs;
  Entity* entities;
  }Node;
  
The entities field is a pointer to an array of elements in this node. The user can independently upgrade the Entity structure in the Entity.h header file (Change the Data structure).
After creating a tree using the create_tree(Btree* tree) function and finishing working with it, you need to call the destroy_tree(Btree* tree) function to free up memory from it.
The maximum number of elements that can be in a node is set in the T variable.
  #define T 3

Work with shared memory is carried out by writing/reading the required node using the push_node_shared_memory() / pull_node_shared_memory() functions. Similarly, for writing/reading individual elements. The sizes of shared memory are set by the constants SHRM_NODE_MAX_SIZE and SHRM_ENTITY_MAX_SIZE. The constant SHRM_NODE_MAX_SIZE directly depends on the specified size T .

Client-server interaction is organized using local sockets with synchronous I/O. The server can support the number of clients that is specified in the constant:
  #define MAX_CLIENT_CONNECT 3
If all possible number of clients are already connected to the server, then a message about server occupancy will be sent to new clients during the connection, after which they process it and finish their work. Otherwise, the server, which is constantly listening to the channel in its main stream, creates a new stream and transfers the client to the client_wrapper() function for processing in this stream. The client_wrapper() function creates shared memory for a specific thread. The first shared memory is created for writing/reading tree nodes, and the second for writing/reading entities. After successful creation of shared memory, the server thread begins to wait for the transmitted command from the client. After receiving, it is processed and, if a command from outside about the completion of the process (Cntr+C) came to the server, it sends the client a command to close the server, otherwise the same command that the user entered is recovered. If the client receives a command to terminate the server, then it cleans up all the data that it has created and completes correctly. The server, when processing the completion of the process, waits for the completion of all its threads, cleans up the database (tree) that is stored on it, deletes shared memory and terminates. If the server has not received a completion command from outside, then it processes the user's command, namely:
  1) Print - blocks all threads, reads the tree, sends it to the client, unlocks all threads. The client displays the transmitted string on the screen.
  2) Get - blocks all threads, searches for the desired node in the tree, puts it in shared memory, unlocks all threads. The client reads and outputs to the screen.
  3) Add - the client enters data about the entity and puts it in shared memory. After that, the server reads it and adds it to the tree.
  4) Remove - the client enters the entity key and passes it to the server, which searches for an entity with this key in the tree and deletes it.
  5) Exit - disconnects the client from the server. The shared memory is destroyed, the client's memory is cleared and the current connection is closed. In the main thread, space is being freed up in the array of clients.

When adding an entity to the database, the client enters a key, which is a regular string. After that, the function of hashing the key and converting it into an unsigned integer is called, which will be used in the future.

# Screenshots
Demonstration of the program:
![image](https://user-images.githubusercontent.com/41357381/149163015-8076e1e3-c08a-4857-b618-79e2d59ce900.png)
![image](https://user-images.githubusercontent.com/41357381/149163502-a2877760-c68b-4ce0-a56a-cdc5597e3896.png)
![image](https://user-images.githubusercontent.com/41357381/149163760-3c10e8b6-1368-4cee-9e02-a2c301611cc4.png)
![image](https://user-images.githubusercontent.com/41357381/149164291-ae5f9a4e-ad77-4df6-a307-97290c86a009.png)
![image](https://user-images.githubusercontent.com/41357381/149164395-09894ced-f46b-473e-94a6-bb877d93ec0d.png)
