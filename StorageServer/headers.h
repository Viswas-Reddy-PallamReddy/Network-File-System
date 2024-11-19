#ifndef HEADERS_H
#define HEADERS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#include <pthread.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/tcp.h>

extern int final_nm_socket;

#define CHUNK_SIZE 256
#define BUFFER_SIZE 4096
#define MAX_ACCESSIBLE_PATHS 100
#define MAX_PATH_LEN 4096

typedef struct
{
    char ip[INET_ADDRSTRLEN];
    int nm_port;
    int client_port;
    int num_accessible_paths;
    char accessible_paths[MAX_ACCESSIBLE_PATHS][MAX_PATH_LEN];
    // int storage_server_number;
} StorageServer;

extern StorageServer this;

typedef struct packet {
    int seq_num;            // Sequence number of the packet
    int total_chunks;       // Total number of chunks
    char data[CHUNK_SIZE];  // Data chunk
} packet;


#endif