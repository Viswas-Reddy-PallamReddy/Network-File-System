#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_PORT 3388
#define SERVER_IP "127.0.0.1" // Change this if testing on different machines
#define MAX_ACCESSIBLE_PATHS 100

typedef struct
{
    char ip[INET_ADDRSTRLEN];
    int server_port;
    int client_port;
    int num_accessible_paths;
    char * accessible_paths[MAX_ACCESSIBLE_PATHS];
    int storage_server_number;
} StorageServer;

int main()
{
    int client_socket;
    struct sockaddr_in server_addr;
    StorageServer storage_server;

    // Fill the StorageServer struct with sample data
    strcpy(storage_server.ip, "192.168.1.100");
    storage_server.server_port = 8080;
    storage_server.client_port = 12345;
    storage_server.num_accessible_paths = 3;
    storage_server.accessible_paths[0] = "/path/1_4";
    storage_server.accessible_paths[1] = "/path/2_4";
    storage_server.accessible_paths[2] = "/path/3_4";

    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0)
    {
        perror("Invalid address/ Address not supported");
        close(client_socket);
        return 1;
    }

    // Connect to the server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connection to the server failed");
        close(client_socket);
        return 1;
    }

    send(client_socket, storage_server.ip, INET_ADDRSTRLEN, 0);
    send(client_socket, &storage_server.server_port, sizeof(int), 0);
    send(client_socket, &storage_server.client_port, sizeof(int), 0);
    send(client_socket, &storage_server.num_accessible_paths, sizeof(int), 0);
    for (int i = 0; i < storage_server.num_accessible_paths; i++)
    {
        int path_len = strlen(storage_server.accessible_paths[i]) +1 ;
        send(client_socket,&path_len,sizeof(int),0);    
        usleep(1000);
        send(client_socket, storage_server.accessible_paths[i], path_len, 0);
    }   

    // Close the socket
    close(client_socket);
    return 0;
}
