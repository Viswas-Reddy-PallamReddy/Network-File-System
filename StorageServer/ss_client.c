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
        // close(client_socket);
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
    char* temp = strtok(NULL, " ");
    char filename[MAX_PATH_LEN];
    snprintf(filename, sizeof(filename), "Paths/%s", temp);
    if (strcmp(command, "READ") == 0) {
        read_file(client_socket, filename);
        close(client_socket);
    }
    else if (strcmp(command, "GET_INFO") == 0) {
        send_file_info(client_socket, filename);
        close(client_socket);
    } else if (strcmp(command, "STREAM") == 0) {
        printf("Streaming audio... %s\n",command);
        stream_audio(client_socket, filename);
        close(client_socket);
    } else if (strcmp(command, "WRITE") == 0) {
        char* token = strtok(NULL, " ");
        int is_sync = 0;
        if(token != NULL && strcmp(token, "--SYNC") == 0){
            is_sync =1;
        }
        write_to_file(client_socket, filename, is_sync);
    } else if (strcmp(command, "EXIT") == 0) {
        close(client_socket);
    }
    // else if(strcmp(command, "LIST") == 0){
    //     print_paths();
    // }
    else {
        // send(client_socket, "Invalid command\n", 16, 0);
        int code = 1;
        send(final_nm_socket, &code, sizeof(int), 0);
        close(client_socket);
    }
}

void read_file(int client_socket, const char *filename) {
    printf("Reading file: %s\n", filename);
    FILE *file = fopen(filename, "r");
    // if (!file) {
    //     packet pkt;
    //     pkt.seq_num = -1;
    //     pkt.total_chunks = 1;
    //     strcpy(pkt.data, "File not found\n");
    //     send(client_socket, pkt.data, strlen(pkt.data), 0);
    //     // send(client_socket, &pkt, sizeof(pkt), 0);
    //     return;
    // }

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
    // send(client_socket, &pkt, sizeof(pkt), 0);
    int code = 0;
    send(client_socket, &code, sizeof(int), 0);
    printf("file size: %ld\n", file_size);
    fclose(file);
}

void write_to_file(int client_socket, const char *filename,int is_sync) {
    packet pkt;
    FILE *file = fopen(filename, "r");
    if (!file) {
        memset(&pkt, 0, sizeof(pkt));
        strcpy(pkt.data, "ERROR 16\n");
        pkt.seq_num = -1;
    //     send(client_socket, &pkt, sizeof(pkt), 0);
        send(client_socket, pkt.data, strlen(pkt.data), 0);
        return;
    }
    fclose(file);
    file = fopen(filename, "a");
    if (!file) {
        memset(&pkt, 0, sizeof(pkt));
        strcpy(pkt.data, "ERROR 9\n");
        pkt.seq_num = -1;
        // send(client_socket, &pkt, sizeof(pkt), 0);
        send(client_socket, pkt.data, strlen(pkt.data), 0);
        return;
    }
    memset(&pkt, 0, sizeof(pkt));
    strcpy(pkt.data, "Enter text to write to the file (type $STOP to end):\n");
    pkt.seq_num = 1;
    // send(client_socket, &pkt, sizeof(pkt), 0);
    printf("Waiting for packets\n");
    send(client_socket, pkt.data, strlen(pkt.data), 0);
    packet pkt_write[MAX_CHUNK_WRITE];
    int i=0;
    for(i=0;i<MAX_CHUNK_WRITE;i++)
    {
        memset(&pkt_write[i],0,sizeof(pkt_write[i]));
        int bytes_received = recv(client_socket, &pkt_write[i], sizeof(pkt_write[i]), 0);
        if (bytes_received <= 0) break;
        printf("Received packet with data: %s\n",pkt_write[i].data);
        if(pkt_write[i].seq_num == -1)
            break;
    }
    if(i+1>THRESHOLD && !is_sync)
    {
        // send(final_nm_socket,"request has been accepted",strlen("request has been accepted"),0);
        send(client_socket,"request has been accepted",strlen("request has been accepted"),0);
        close(client_socket);
    }
    printf("All packets received\n");
    for(int j=0;j<i;j++)
    {
        
        fwrite(pkt_write[j].data,1,strlen(pkt_write[j].data),file);
    }

    int code = 0;
    printf("File modified successfully\n");
    // send(client_socket,&code, sizeof(int), 0);
    send(final_nm_socket,"ERROR 0", 10, 0);
    send(client_socket,"ERROR 0", 10, 0);

    fclose(file);
}

void send_file_info(int client_socket, const char *filename) {
    struct stat file_stat;
    if (stat(filename, &file_stat) < 0) {
        // send(client_socket, "File not found\n", 15, 0);
        int code = 16;
        send(client_socket, &code, sizeof(int), 0);
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
    // if(fd < 0){
    //     // printf("Couldnt open file\n");
    //     int code = 10;
    //     send(client_socket, &code, sizeof(int), 0);
    // }
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

// void traverse_directory1(const char* dirname,const char* base,char* temp,int* num_accessible_paths){
//     DIR *dir;
//     struct dirent *ent;
//     struct stat st;

//     // Open the directory
//     if ((dir = opendir(dirname)) != NULL) {
//         while ((ent = readdir(dir)) != NULL) {
//             if (ent->d_name[0] == '.') {
//                 continue;  // Skip hidden files and directories (e.g., "." and "..")
//             }

//             char full_path[1024];
            
//             snprintf(full_path, sizeof(full_path), "%s/%s", dirname, ent->d_name);

//             // Get the file status
//             if (stat(full_path, &st) == 0) {
//                 char relative_path[1024];

//                 // Calculate the relative path
//                 if (strcmp(base, ".") == 0) {
//                     snprintf(relative_path, sizeof(relative_path), "%s", ent->d_name);
//                 } else {
//                     snprintf(relative_path, sizeof(relative_path), "./%s", full_path + strlen(base) + 1);
//                 }

//                 // Recursive call for subdirectories
//                 if (S_ISDIR(st.st_mode)) {
//                     strcat(relative_path, " folder");
//                     traverse_directory1(full_path, base,temp,num_accessible_paths);
//                 }
//                 else{
//                     strcat(relative_path, " file");
//                 }
//                 // strcpy(this.accessible_paths[this.num_accessible_paths], relative_path);
//                 strcat(temp,relative_path);
 
//                     strcat(temp,"\n");
//                 (*num_accessible_paths)++;
//                 // this.num_accessible_paths++;
//             }
//         }
//         closedir(dir);
//     } else {
//         perror("Error reading directory");
//     }
// }

// void print_paths() {
//     char base[1024];
//     getcwd(base, sizeof(base));
//     printf("base dir: %s\n",base);
//     strcat(base,"/Paths");
//     int num_accessible_paths = 0;
//     char temp[MAX_PATH_LEN*100];
//     traverse_directory1(base, base,temp,&num_accessible_paths);
//     printf("num_accessible_paths: %d\n",num_accessible_paths);
//     // for(int i = 0; i < num_accessible_paths; i++){
//         printf("%s\n",temp);
//     // }
// }
