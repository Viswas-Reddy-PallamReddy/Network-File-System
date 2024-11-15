#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <time.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void handle_client(int client_socket);
void create_file(int client_socket, const char *filename);
void delete_file(int client_socket, const char *filename);
void send_file_data(int client_socket, const char *filename);
void send_file_info(int client_socket, const char *filename);
void stream_audio(int client_socket, const char *filename);
void write_to_file(int client_socket, const char *filename);

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Bind the socket to the port
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_socket, 5) < 0) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d...\n", PORT);

    while (1) {
        // Accept client connection
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_len);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }
        printf("Client connected.\n");

        // Handle client request
        handle_client(client_socket);
        close(client_socket);
        printf("Client disconnected.\n");
    }

    close(server_socket);
    return 0;
}

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_received <= 0) return;

    buffer[bytes_received] = '\0';
    char command[10], filename[100];
    sscanf(buffer, "%s %s", command, filename);

    if (strcmp(command, "read") == 0) {
        send_file_data(client_socket, filename);
    } else if (strcmp(command, "get_info") == 0) {
        send_file_info(client_socket, filename);
    } else if (strcmp(command, "stream_audio") == 0) {
        stream_audio(client_socket, filename);
    } else if (strcmp(command, "write") == 0) {
        write_to_file(client_socket, filename);
    } else if (strcmp(command, "delete") == 0) {
        delete_file(client_socket, filename);
    }
    else if (strcmp(command, "create") == 0) {
        // create_file(client_socket,path, filename);
        create_file(client_socket, filename);
    }
    else {
        send(client_socket, "Invalid command\n", 16, 0);
    }
}

void send_file_data(int client_socket, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        send(client_socket, "File not found\n", 15, 0);
        return;
    }

    char buffer[BUFFER_SIZE];
    while (fgets(buffer, BUFFER_SIZE, file)) {
        send(client_socket, buffer, strlen(buffer), 0);
    }

    fclose(file);
}

void delete_file(int client_socket, const char *filename){
    char buffer[BUFFER_SIZE];
    if (remove(filename) == 0) {
        snprintf(buffer, sizeof(buffer), "Deleted file: %s successfully\n", filename);
        send(client_socket, buffer, strlen(buffer), 0);
    } else {
        send(client_socket, "Failed to delete file\n", 22, 0);
    }
}

void create_file(int client_socket, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        send(client_socket, "Failed to create file\n", 23, 0);
        return;
    }

    fclose(file);
}

void send_file_info(int client_socket, const char *filename) {
    struct stat file_stat;
    if (stat(filename, &file_stat) < 0) {
        send(client_socket, "File not found\n", 15, 0);
        return;
    }

    char info[512];
    snprintf(info, sizeof(info),
             "Size: %ld bytes\nPermissions: %o\nLast accessed: %sLast modified: %s",
             file_stat.st_size, file_stat.st_mode & 0777,
             ctime(&file_stat.st_atime), ctime(&file_stat.st_mtime));

    send(client_socket, info, strlen(info), 0);
}

void stream_audio(int client_socket, const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        send(client_socket, "File not found\n", 15, 0);
        return;
    }

    char buffer[BUFFER_SIZE];
    int bytes_read;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        send(client_socket, buffer, bytes_read, 0);
    }

    fclose(file);

}

void write_to_file(int client_socket, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        send(client_socket, "Failed to create file\n", 23, 0);
        return;
    }

    char buffer[BUFFER_SIZE];
    int bytes_received;
    while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        fwrite(buffer, 1, bytes_received, file);
    }
    fclose(file);
}
