#ifndef NAMING_SERVER_H
#define NAMING_SERVER_H

#include <stdio.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BUFFER_SIZE 5000
#define MAX_STORAGE_SERVERS 20
#define MAX_CLIENTS 200
#define NM_PORT 3388
#define CLIENT_PORT 12345
#define MAX_FILES_PER_STORAGE_SERVER 100
#define MAX_ACCESSIBLE_PATHS 100
#define INITIAL_STORAGE_SERVERS 2
#define MAX_FILE_NAME_SIZE 100

typedef struct
{
    char ip[INET_ADDRSTRLEN];
    int server_port;
    int client_port;
    int num_accessible_paths;
    char accessible_paths[MAX_ACCESSIBLE_PATHS][MAX_FILE_NAME_SIZE];
    int storage_server_number;
} StorageServer;



typedef struct TrieNode
{
    struct TrieNode *children[256];
    StorageServer *server;
    int isEndOfPath; // Added to mark end of valid paths
} TrieNode;

TrieNode *create_trie_node();
void insert_path(TrieNode *root, const char *path, StorageServer *server);
StorageServer *find_storage_server(TrieNode *root, const char *path);


#endif