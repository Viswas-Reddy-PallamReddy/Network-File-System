#include "headers.h"
#include "SS_client.h"
#include "SS_NM.h"


#define CLIENT_PORT 8080
#define NM_PORT 8081
#define BUFFER_SIZE 4096
#define CHUNK_SIZE 1024  // Define the size of each data chunk

typedef struct packet {
    int seq_num;            // Sequence number of the packet
    int total_chunks;       // Total number of chunks
    char data[CHUNK_SIZE];  // Data chunk
} packet;

StorageServer this;

void handle_client(int client_socket);
void create_file(int client_socket, const char *filepath);
void create_folder(int client_socket,const char* filepath);
void delete_file(int client_socket, const char *filepath);
void delete_folder(int client_socket,const char* filepath);
void send_file_data(int client_socket, const char *filename);
void send_file_info(int client_socket, const char *filename);
void stream_audio(int client_socket, const char *filename);
void write_to_file(int client_socket, const char *filename);
void traverse_directory(const char *dirname, const char *base);
void list_all_accessible_files();

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
    server_addr.sin_port = htons(CLIENT_PORT);

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

    printf("Server is listening on port %d...\n", CLIENT_PORT);

    while (1) {
        // Accept client connection
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_len);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }
        printf("Client connected.\n");

        // Handle client request
        // handle_client(client_socket);
        handle_nm(client_socket);
        close(client_socket);
        printf("Client disconnected.\n");
    }

    close(server_socket);
    return 0;
}

void traverse_directory(const char *dirname, const char *base) {
    DIR *dir;
    struct dirent *ent;
    struct stat st;

    // Open the directory
    if ((dir = opendir(dirname)) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (ent->d_name[0] == '.') {
                continue;  // Skip hidden files and directories (e.g., "." and "..")
            }

            char full_path[1024];
            snprintf(full_path, sizeof(full_path), "%s/%s", dirname, ent->d_name);

            // Get the file status
            if (stat(full_path, &st) == 0) {
                char relative_path[1024];

                // Calculate the relative path
                if (strcmp(base, ".") == 0) {
                    // If the base is ".", use only the file/folder name
                    snprintf(relative_path, sizeof(relative_path), "%s", ent->d_name);
                } else {
                    // Otherwise, construct the relative path by removing the base length from the full path
                    snprintf(relative_path, sizeof(relative_path), "./%s", full_path + strlen(base) + 1);
                }

                // Recursive call for subdirectories
                if (S_ISDIR(st.st_mode)) {
                    traverse_directory(full_path, base);
                }
                strcpy(this.accessible_paths[this.num_accessible_paths], relative_path);
                // Print the relative path
                // printf("Relative Path: %s\n", relative_path);
                this.num_accessible_paths++;
            }
        }
        closedir(dir);
    } else {
        perror("Error reading directory");
    }
}

void list_all_accessible_files(){
    char base[1024];
    getcwd(base, sizeof(base));
    traverse_directory(base, base);
}

void initialise_to_nm(int client_socket){
    // // this = (StorageServer*)malloc(sizeof(StorageServer));
    // // if(this == NULL){
    // //     perror("Failed to allocate memory for StorageServer");
    // //     exit(EXIT_FAILURE);
    // }
    this.nm_port = NM_PORT;
    this.client_port = CLIENT_PORT;
    this.num_accessible_paths = 0;
    for(int i = 0; i < this.num_accessible_paths; i++){
        strcpy(this.accessible_paths[i],"");
    }
    char base[1024];
    getcwd(base, sizeof(base));
    printf("base dir: %s\n",base);
    traverse_directory(base, base);
    printf("num_accessible_paths: %d\n",this.num_accessible_paths);
    for(int i = 0; i < this.num_accessible_paths; i++){
        printf("%s\n",this.accessible_paths[i]);
    }
    printf("num_accessible_paths: %d\n",this.num_accessible_paths);
    send(client_socket, &this,sizeof(StorageServer),0);
}

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_received <= 0) return;

    buffer[bytes_received] = '\0';
    printf("Received command: %s\n", buffer);
    char command[20], filename[100];
    sscanf(buffer, "%s %s", command, filename);
    if (strcmp(command, "read") == 0) {
        send_file_data(client_socket, filename);
    }
    // else if(strcmp(command, "list") == 0){
    //     initialise_to_nm(client_socket);
    // }
    else if (strcmp(command, "get_info") == 0) {
        send_file_info(client_socket, filename);
    } else if (strcmp(command, "stream") == 0) {
        printf("Streaming audio... %s\n",command);
        stream_audio(client_socket, filename);
    } else if (strcmp(command, "write") == 0) {
        write_to_file(client_socket, filename);
    } else {
        send(client_socket, "Invalid command\n", 16, 0);
    }
}

void handle_nm(int client_socket){
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_received <= 0) return;

    buffer[bytes_received] = '\0';
    printf("Received command: %s\n", buffer);
    char* command = strtok(buffer, " ");
    printf("command: %s\n",command);

    if(strcmp(command, "create")==0){
        char* path = strtok(NULL, " ");
        printf("path: %s\n",path);
        char* filename = strtok(NULL, " ");
        printf("filename: %s\n",filename);
        char full_path[MAX_PATH_LEN];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, filename);
        send(client_socket,"Do you want to create a file or folder? (file/folder):", 54, 0);
        char type[10];
        memset(type, 0, 10);
        int bytes_received = recv(client_socket, type, 10, 0);
        if (bytes_received <= 0) return;
        type[bytes_received] = '\0';
        printf("type: %s\n",type);
        if(strcmp(type, "file")==0){
            create_file(client_socket, full_path);
        }
        else if(strcmp(type, "folder")==0){
            create_folder(client_socket, full_path);
        }
    }

    else if(strcmp(command, "delete")==0){
        char* filename = strtok(NULL, " ");
        printf("filename: %s\n",filename);
        DIR* dir = opendir(filename);
        if (dir == NULL) {
            delete_file(client_socket,filename);
        } else {
            delete_folder(client_socket,filename);
        }
        closedir(dir);
    }

}
void send_file_data(int client_socket, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        send(client_socket, "File not found\n", 15, 0);
        return;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Calculate total number of chunks
    int total_chunks = (file_size + CHUNK_SIZE - 1) / CHUNK_SIZE;

    packet pkt;
    int bytes_read;
    for (int seq_num = 0; seq_num < total_chunks; seq_num++) {
        pkt.seq_num = seq_num;
        pkt.total_chunks = total_chunks;
        memset(pkt.data, 0, CHUNK_SIZE);
        bytes_read = fread(pkt.data, 1, CHUNK_SIZE, file);
        printf("packet data: %s\n",pkt.data);
        // Send packet data
        send(client_socket, &pkt, sizeof(pkt), 0);
    }
    printf("file size: %ld\n", file_size);
    fclose(file);
}

void delete_file(int client_socket, const char *filename) {
    char buffer[BUFFER_SIZE];
    if (remove(filename) == 0) {
        snprintf(buffer, sizeof(buffer), "Deleted file: %s successfully\n", filename);
        send(client_socket, buffer, strlen(buffer), 0);
    } else {
        send(client_socket, "Failed to delete file\n", 22, 0);
    }
}

void create_file(int client_socket, const char *filepath) {
    printf("Creating file: %s\n", filepath);
    FILE *file = fopen(filepath, "w");
    if (!file) {
        send(client_socket, "Failed to create file\n", 23, 0);
        return;
    }
    send(client_socket, "File created successfully\n", 26, 0);
    fclose(file);
}

void create_folder(int client_socket, const char* filepath){
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    int status = mkdir(filepath, 0777);
    if (status == -1) {
        snprintf(buffer, sizeof(buffer), "Failed to create folder %s\n", filepath);
        send(client_socket, buffer, strlen(buffer), 0);
        printf("Error creating folder %s\n", filepath);
    } else {
        snprintf(buffer, sizeof(buffer), "Folder %s created successfully\n", filepath);
        send(client_socket, buffer, strlen(buffer), 0);
        printf("Folder %s created successfully\n", filepath);
    }
}

void delete_folder(int client_socket, const char* filepath){
    DIR* dir = opendir(filepath);
    if (dir == NULL) {
        printf("Error deleting folder %s\n", filepath);
        return;
    }

    struct dirent* ent;
    while ((ent = readdir(dir)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
            continue;
        }

        char full_path[MAX_PATH_LEN];
        sprintf(full_path, "%s/%s", filepath, ent->d_name);
        if (ent->d_type == DT_DIR) {
            delete_folder(client_socket,full_path);
        } else {
            delete_file(client_socket,full_path);
        }
    }

    closedir(dir);
    int status = rmdir(filepath);
    char buffer[BUFFER_SIZE];
    if (status == -1) {
        snprintf(buffer, sizeof(buffer), "Failed to delete folder %s\n", filepath);
        send(client_socket, buffer, strlen(buffer), 0);
        printf("Error deleting folder %s\n", filepath);
    } else {
        snprintf(buffer, sizeof(buffer), "Folder %s deleted successfully\n", filepath);
        send(client_socket, buffer, strlen(buffer), 0);
        printf("Folder %s deleted successfully\n", filepath);
    }
}

void send_file_info(int client_socket, const char *filename) {
    struct stat file_stat;
    if (stat(filename, &file_stat) < 0) {
        send(client_socket, "File not found\n", 15, 0);
        return;
    }

    // Prepare file info as a string
    char info[512];
    memset(info, 0, sizeof(info));
    snprintf(info, sizeof(info),
             "Size: %ld bytes\nPermissions: %o\nLast accessed: %sLast modified: %s",
             file_stat.st_size, file_stat.st_mode & 0777,
             ctime(&file_stat.st_atime), ctime(&file_stat.st_mtime));

    // Send file info in packets
    packet pkt;
    int total_chunks = (strlen(info) + CHUNK_SIZE - 1) / CHUNK_SIZE;
    memset(pkt.data, 0, CHUNK_SIZE);
    strcpy(pkt.data, info);
    pkt.seq_num = 0;
    pkt.total_chunks = 1;
    send(client_socket, &pkt, sizeof(pkt), 0);
}

void stream_audio(int client_socket, const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        send(client_socket, "File not found\n", 15, 0);
        return;
    }
    long file_size = 0;
    // // Get file size
    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    printf("file size: %ld\n", file_size);
    // Calculate total number of chunks
    int total_chunks = (file_size + CHUNK_SIZE - 1) / CHUNK_SIZE;
    packet pkt;
    int bytes_read;
    for (int seq_num = 0; seq_num < total_chunks; seq_num++) {
        pkt.seq_num = seq_num;
        pkt.total_chunks = total_chunks;
        memset(pkt.data, 0, CHUNK_SIZE);
        bytes_read = fread(pkt.data, 1, CHUNK_SIZE, file);

        // Send packet data
        send(client_socket, &pkt, sizeof(pkt), 0);
    }

    fclose(file);
}

void write_to_file(int client_socket, const char *filename) {
    FILE *file = fopen(filename, "a");
    if (!file) {
        send(client_socket, "Failed to create file\n", 23, 0);
        return;
    }

    packet pkt;
    int bytes_received;
    while ((bytes_received = recv(client_socket, &pkt, sizeof(pkt), 0)) > 0) {
        if (bytes_received == sizeof(pkt) && pkt.seq_num != -1) {
            printf("packet data: %s\n",pkt.data);
            fwrite(pkt.data, 1, strlen(pkt.data), file);
        }
    }
    fclose(file);
}
