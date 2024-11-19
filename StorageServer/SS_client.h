#ifndef SS_CLIENT_H
#define SS_CLIENT_H

#include "headers.h"

#define CLIENT_PORT 7223
#define MAX_CHUNK_WRITE 40
#define THRESHOLD 5


// void connect_to_client();
void *handle_client_thread(void *arg);
void handle_client(int client_socket);
void read_file(int client_socket, const char *filename);
void send_file_info(int client_socket, const char *filename);
void stream_audio(int client_socket, const char *filename);
void write_to_file(int client_socket, const char *filename,int is_sync);
// void traverse_directory1(const char* dirname,const char* base,char* temp,int* num_accessible_paths);
// void print_paths();




#endif