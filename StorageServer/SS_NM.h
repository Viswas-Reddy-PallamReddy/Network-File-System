#ifndef SS_NM_H
#define SS_NM_H

#include "headers.h"
#include "SS_client.h"

int connect_to_nm(int nm_port,const char* ip_address);
void initialise_to_nm(int nm_socket,const char* ip_address);
void *process_requests(void *arg);
void *handle_nm_thread(void *arg);
void handle_nm(char *nm_command);
void create_file( const char *filepath);
void delete_file(const char *filepath);
void create_folder(const char* filepath);
void delete_folder(const char* filepath);
void traverse_directory(const char *dirname, const char *base);

#endif