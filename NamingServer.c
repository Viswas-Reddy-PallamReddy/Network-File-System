#include <stdio.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE 2560
#define MAX_STORAGE_SERVERS 15
#define MAX_CLIENTS 150
#define NM_PORT 3388
#define CLIENT_PORT 12345
#define MAX_FILES_PER_STORAGE_SERVER 100

typedef struct
{
    char ip[INET_ADDRSTRLEN];
    int server_port;
    int client_port;
    char **accessible_paths;
} StorageServer;

StorageServer storage_servers[MAX_STORAGE_SERVERS];

void initialze_storage_server()
{
    // so i am assuming that SS will send a struct to me
    // and i will store it in a array of storage servers
    /// need to think when to stop the server
    struct sockaddr_in nm_server_addr;
    int nm_server_addr_len = sizeof(nm_server_addr);
    int nm_server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (nm_server_socket < 0)
    {
        perror("Error in creating naming server socket");
        exit(1);
    }
    nm_server_addr.sin_family = AF_INET;
    nm_server_addr.sin_port = htons(NM_PORT);
    nm_server_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(nm_server_socket, (struct sockaddr *)&nm_server_addr, sizeof(nm_server_addr)) < 0)
    {
        perror("Error in binding naming server socket");
        exit(1);
    }
    int listen_status = listen(nm_server_socket, MAX_STORAGE_SERVERS);
    if (listen_status < 0)
    {
        perror("Error in listening to naming server socket");
        exit(1);
    }

    int storage_server_count = 0;
    while (1)
    {
        int accept_status = accept(nm_server_socket, (struct sockaddr *)&nm_server_addr, &nm_server_addr_len);
        if (accept_status < 0)
        {
            perror("Error in accepting connection from storage server");
            exit(1);
        }
        StorageServer new_storage_server;
        int bytes_received = recv(accept_status, &new_storage_server, sizeof(new_storage_server), 0);
        if (bytes_received < 0)
        {
            perror("Error in receiving data from storage server");
            exit(1);
        }
        storage_servers[storage_server_count++] = new_storage_server;
        printf("Registered new storage server: %s\n", new_storage_server.ip);
        printf("Registered storage server Port for client : %d\n", new_storage_server.client_port);
        printf("Registered storage server Port for NM : %d\n", new_storage_server.server_port);
        // printf("Registered storage server accessible paths : %s\n", new_storage_server.accessible_paths);
    }
}

int Send_storage_server_details()
{
    // send storage server details to client
}

void file_path(char *path)
{
    printf("File path is : %s\n", path);
    // return Storage server details to client
    int details_of_storage_server = Send_storage_server_details();
}

void print_all_accessible_paths()
{
    // print all accessible paths
    for (int i = 0; i < MAX_STORAGE_SERVERS; i++)
    {
        printf("Storage server %d accessible paths are : %s\n", i);
        for (int j = 0; j < MAX_FILES_PER_STORAGE_SERVER; j++)
        {
            printf("%s\n", storage_servers[i].accessible_paths[j]);
        }
    }
}

void *process_client_requests(void *accept_status)
{
    /// assuming that client will send a string request to me
    int accept_status_1 = *(int *)accept_status;
    char buffer[BUFFER_SIZE];
    int bytes_received = recv(accept_status_1, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received < 0)
    {
        perror("Error in receiving data from client");
        exit(1);
    }
    buffer[bytes_received] = '\0';
    printf("Received request from client: %s\n", buffer);
    const char *response = "ACK: Request received\n";
    send(accept_status_1, response, strlen(response), 0);
    if (strncmp("buffer", "READ", 4) == 0)
    {
        file_path(buffer + 5);
    }
    else if (strncmp("buffer", "WRITE", 5) == 0)
    {
        file_path(buffer + 6);
    }
    else if (strncmp("buffer", "DELETE", 6) == 0)
    {
        file_path(buffer + 7);
    }
    else if (strncmp("buffer", "CREATE", 6) == 0)
    {
        file_path(buffer + 7);
    }
    else if (strncmp("buffer", "LIST", 4) == 0)
    {
        print_all_accessible_paths();
    }
    else if (strncmp("buffer", "ADDITIONAL_INFO", 15) == 0)
    {
        // exit(1);
    }
    else if (strncmp("buffer", "AUDIO_FILES/STREAM", 11) == 0)
    {
        // exit(1);
    }
    else if (strncmp("buffer", "COPY", 4) == 0)
    {
    }
    else
    {
        printf("Invalid request from client\n");
    }
    return NULL;
}

void Handle_client_requests()
{
    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0)
    {
        perror("Error in creating client socket");
        exit(1);
    }
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(CLIENT_PORT);
    client_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(client_socket, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0)
    {
        perror("Error in binding client socket");
        exit(1);
    }
    int listen_status = listen(client_socket, MAX_CLIENTS);
    if (listen_status < 0)
    {
        perror("Error in listening to client socket");
        exit(1);
    }
    while (1)
    {
        printf("Waiting for client connection\n");
        int accept_status = accept(client_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (accept_status < 0)
        {
            perror("Error in accepting connection from client");
            exit(1);
        }
        pthread_t thread;
        pthread_create(&thread, NULL, process_client_requests, &accept_status);
        pthread_detach(thread);
    }
}

int main()
{
    // initialze_storage_server();
    Handle_client_requests();
    sleep(100);
    return 0;
}