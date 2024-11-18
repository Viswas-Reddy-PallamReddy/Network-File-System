#ifndef SS_CLIENT_H
#define SS_CLIENT_H

#include "headers.h"

#define MAX_ACCESSIBLE_PATHS 100
#define MAX_PATH_LEN 1024

typedef struct
{
    char ip[INET_ADDRSTRLEN];
    int nm_port;
    int client_port;
    int num_accessible_paths;
    char accessible_paths[MAX_ACCESSIBLE_PATHS][MAX_PATH_LEN];
} StorageServer;

extern StorageServer this;

void connect_to_client();
void handle_client(int client_socket);
void send_file_data(int client_socket, const char *filename);
void send_file_info(int client_socket, const char *filename);
void stream_audio(int client_socket, const char *filename);
void write_to_file(int client_socket, const char *filename);

#endif