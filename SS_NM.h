#ifndef SS_NM_H
#define SS_NM_H

#include "headers.h"
#include "SS_client.h"

void initialise_to_nm(int client_socket);
void handle_nm(int client_socket);
void create_file(int client_socket, const char *filepath);
void delete_file(int client_socket, const char *filepath);
void create_folder(int client_socket,const char* filepath);
void delete_folder(int client_socket,const char* filepath);


#endif