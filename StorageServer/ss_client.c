#include "headers.h"
#include "SS_client.h"

void *handle_client_thread(void *arg) {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        pthread_exit(NULL);
    }

    // Bind the socket to the port
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(CLIENT_PORT);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        pthread_exit(NULL);
    }

    // Listen for client connections
    if (listen(server_socket, 5) < 0) {
        perror("Listen failed");
        close(server_socket);
        pthread_exit(NULL);
    }

    printf("Waiting for client on port %d...\n", CLIENT_PORT);

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_len);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        printf("Client connected.\n");
        handle_client(client_socket);
        close(client_socket);
        printf("Client disconnected.\n");
    }

    close(server_socket);
    pthread_exit(NULL);
}

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_received <= 0) return;

    buffer[bytes_received] = '\0';
    printf("Received command: %s\n", buffer);
    char* command = strtok(buffer, " ");
    char* filename = strtok(NULL, " ");
    if (strcmp(command, "READ") == 0) {
        read_file(client_socket, filename);
    }
    else if (strcmp(command, "GET_INFO") == 0) {
        send_file_info(client_socket, filename);
    } else if (strcmp(command, "STREAM") == 0) {
        printf("Streaming audio... %s\n",command);
        stream_audio(client_socket, filename);
    } else if (strcmp(command, "WRITE") == 0) {
        write_to_file(client_socket, filename);
    } else if (strcmp(command, "EXIT") == 0) {
        close(client_socket);
    }
    else {
        send(client_socket, "Invalid command\n", 16, 0);
    }
}

void read_file(int client_socket, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        packet pkt;
        pkt.seq_num = -1;
        pkt.total_chunks = 1;
        strcpy(pkt.data, "File not found\n");
        send(client_socket, &pkt, sizeof(pkt), 0);
        return;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Calculate total number of chunks
    int total_chunks = (file_size + CHUNK_SIZE - 1) / CHUNK_SIZE;
    // total_chunks++;
    printf("total chunks: %d\n", total_chunks);

    packet pkt;
    int bytes_read;
    for (int seq_num = 0; seq_num < total_chunks; seq_num++) {
        memset(&pkt, 0, sizeof(pkt));
        pkt.seq_num = seq_num;
        pkt.total_chunks = total_chunks;
        // memset(pkt.data, 0, CHUNK_SIZE);
        bytes_read = fread(pkt.data, 1, CHUNK_SIZE, file);
        printf("seq_num: %d\n, packet data: %s\n",pkt.seq_num,pkt.data);
        // Send packet data
        send(client_socket, &pkt, sizeof(pkt), 0);
    }
    memset(&pkt, 0, sizeof(pkt));
    pkt.seq_num = -1;
    pkt.total_chunks = 1;
    char* message = "File sent successfully\n";
    // memset(pkt.data, 0, CHUNK_SIZE);
    strcpy(pkt.data, message);
    send(client_socket, &pkt, sizeof(pkt), 0);
    printf("file size: %ld\n", file_size);
    fclose(file);
}

void write_to_file(int client_socket, const char *filename) {
    packet pkt;
    FILE *file = fopen(filename, "r");
    if (!file) {
        memset(&pkt, 0, sizeof(pkt));
        strcpy(pkt.data, "File not found\n");
        pkt.seq_num = -1;
        send(client_socket, &pkt, sizeof(pkt), 0);
        return;
    }
    fclose(file);
    file = fopen(filename, "a");
    if (!file) {
        memset(&pkt, 0, sizeof(pkt));
        strcpy(pkt.data, "Error writing to file\n");
        pkt.seq_num = -1;
        send(client_socket, &pkt, sizeof(pkt), 0);
        return;
    }
    memset(&pkt, 0, sizeof(pkt));
    strcpy(pkt.data, "Enter text to write to the file (type $STOP to end):\n");
    pkt.seq_num = 1;
    send(client_socket, &pkt, sizeof(pkt), 0);
    int bytes_received;
    while ((bytes_received = recv(client_socket, &pkt, sizeof(pkt), 0)) > 0) {
        if (bytes_received == sizeof(pkt) && pkt.seq_num != -1) {
            printf("packet data: %s\n",pkt.data);
            fwrite(pkt.data, 1, strlen(pkt.data), file);
        }
        else if (pkt.seq_num == -1)
            break;
        memset(&pkt, 0, sizeof(pkt));
    }
    send(client_socket,"File modified successfully",strlen("File modified successfully"),0);
    fclose(file);
}

void send_file_info(int client_socket, const char *filename) {
    struct stat file_stat;
    if (stat(filename, &file_stat) < 0) {
        send(client_socket, "File not found\n", 15, 0);
        return;
    }
    char permissions[11];
    snprintf(permissions, sizeof(permissions),
             "%s%s%s%s%s%s%s%s%s%s",
             (S_ISDIR(file_stat.st_mode)) ? "d" : "-",
             (file_stat.st_mode & S_IRUSR) ? "r" : "-",
             (file_stat.st_mode & S_IWUSR) ? "w" : "-",
             (file_stat.st_mode & S_IXUSR) ? "x" : "-",
             (file_stat.st_mode & S_IRGRP) ? "r" : "-",
             (file_stat.st_mode & S_IWGRP) ? "w" : "-",
             (file_stat.st_mode & S_IXGRP) ? "x" : "-",
             (file_stat.st_mode & S_IROTH) ? "r" : "-",
             (file_stat.st_mode & S_IWOTH) ? "w" : "-",
             (file_stat.st_mode & S_IXOTH) ? "x" : "-");
    // Prepare file info as a string
    char info[512];
    memset(info, 0, sizeof(info));
    snprintf(info, sizeof(info),
             "Size: %ld bytes\nPermissions: %s\nLast accessed: %sLast modified: %s",
             file_stat.st_size, permissions,
             ctime(&file_stat.st_atime), ctime(&file_stat.st_mtime));
    send(client_socket, info, strlen(info), 0);
}

void stream_audio(int client_socket, const char *filename) {
    int fd = open(filename,O_RDONLY);
    if(fd < 0){
        printf("Couldnt open file\n");
    }
    int buffer_size = 256;
    char buffer[buffer_size];
    ssize_t bytes_read;

    while ((bytes_read = read(fd, buffer, buffer_size)) > 0) {
        if (send(client_socket, buffer, bytes_read,0) < 0) {
            perror("Failed to send MP3 data");
            close(fd);
            close(client_socket);
            return;
        }
    }

    close(fd);
    close(client_socket);
    printf("MP3 file sent successfully.\n");

    return;
}