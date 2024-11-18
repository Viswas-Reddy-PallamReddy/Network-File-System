#include "NamingServer.h"
struct sockaddr_in nm_server_addr;
socklen_t nm_server_addr_len;
int nm_server_socket;

StorageServer storage_servers[MAX_STORAGE_SERVERS];
TrieNode *trie_root = NULL;
int storage_server_count = 0;
void tokenize(char *str, char *delim, char *tokens[])
{
    char *token = strtok(str, delim);
    int i = 0;
    while (token)
    {
        tokens[i++] = token;
        token = strtok(NULL, delim);
    }
    tokens[i] = NULL;
}

void add_storage_server(int accept_status)
{
    printf("Adding storage server\n");
    StorageServer new_storage_server;

    char temp_1[MAX_FILE_NAME_SIZE];
    int bytes_received_1 = recv(accept_status, temp_1, sizeof(temp_1), 0);
    if (bytes_received_1 < 0)
    {
        perror("Error in receiving data from storage server");
        exit(1);
    }
    // printf("Received storage server details\n");
    // printf("TEMP_1 is : %s\n", temp_1);

    char *temp_3[MAX_FILE_NAME_SIZE];
    tokenize(temp_1, " ", temp_3);
    strcpy(new_storage_server.ip, temp_3[0]);
    new_storage_server.server_port = atoi(temp_3[1]);
    new_storage_server.client_port = atoi(temp_3[2]);
    new_storage_server.num_accessible_paths = atoi(temp_3[3]);
    new_storage_server.file_descriptor = accept_status;
    new_storage_server.storage_server_number = storage_server_count;

    printf("Registered new storage server: %s\n", new_storage_server.ip);
    printf("Registered storage server Port for client : %d\n", new_storage_server.client_port);
    printf("Registered storage server Port for NM : %d\n", new_storage_server.server_port);
    printf("Storage server count is : %d\n", new_storage_server.storage_server_number);
    printf("Registered storage server accessible paths : %d\n", new_storage_server.num_accessible_paths);

    char temp[MAX_FILE_NAME_SIZE];
    int bytes_received = recv(accept_status, temp, sizeof(temp), 0);
    if (bytes_received < 0)
    {
        perror("Error in receiving data from storage server");
        exit(1);
    }
    printf("Received storage server details\n");
    // printf("TEMP is : %s\n", temp);

    char *temp_4[MAX_FILE_NAME_SIZE];
    tokenize(temp, "\n", temp_4);
    // printf("Tokenized\n");

    for (int i = 0; i < new_storage_server.num_accessible_paths; i++)
    {
        // printf("Token %d is : %s\n", i, temp_4[i]);
        strcpy(new_storage_server.accessible_paths[i], temp_4[i]);
        printf("Path is : %s\n", new_storage_server.accessible_paths[i]);
    }
    storage_servers[storage_server_count++] = new_storage_server;

    // for (int i = 0; i < new_storage_server.num_accessible_paths; i++)
    // {
    //     printf("Registered storage server accessible paths : %s\n", new_storage_server.accessible_paths[i]);
    // }

    // Add the accessible paths to the trie
    for (int i = 0; i < new_storage_server.num_accessible_paths; i++)
    {
        insert_path(trie_root, new_storage_server.accessible_paths[i], &storage_servers[storage_server_count - 1]);
    }

    int open_status = open("Log.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (open_status < 0)
    {
        perror("Error in opening file");
        exit(1);
    }
    write(open_status, "Received storage server details\n", strlen("Received storage server details\n"));
    write(open_status, "Registered new storage server: ", strlen("Registered new storage server: "));
    write(open_status, new_storage_server.ip, strlen(new_storage_server.ip));
    write(open_status, "\n", strlen("\n"));
    write(open_status, "Registered storage server Port for client : ", strlen("Registered storage server Port for client : "));
    char client_port[10];
    sprintf(client_port, "%d", new_storage_server.client_port);
    write(open_status, client_port, strlen(client_port));
    write(open_status, "\n", strlen("\n"));
    write(open_status, "Registered storage server Port for NM : ", strlen("Registered storage server Port for NM : "));
    char server_port[10];
    sprintf(server_port, "%d", new_storage_server.server_port);
    write(open_status, server_port, strlen(server_port));
    write(open_status, "\n", strlen("\n"));
    write(open_status, "\n", strlen("\n"));

    // int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    // if (server_socket < 0)
    // {
    //     perror("Error in creating server socket");
    //     exit(1);
    // }
    // struct sockaddr_in server_addr;
    // server_addr.sin_family = AF_INET;
    // server_addr.sin_port = htons(new_storage_server.server_port);
    // // server_addr.sin_addr.s_addr = storage_servers[storage_server_count - 1].ip;
    // if (inet_pton(AF_INET, new_storage_server.ip, &server_addr.sin_addr) <= 0)
    // {
    //     perror("Invalid address/ Address not supported");
    //     close(server_socket);
    //     return;
    // }

    // sleep(5);

    // int con_e = connect(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    // if (con_e < 0)
    // {
    //     perror("Error in connecting to storage server");
    //     exit(1);
    // }

    // printf("SENDING ACK\n");

    // const char *message = "HI\n";
    // ssize_t sent = send(server_socket, message, strlen(message), 0);
    // if (sent < 0)
    // {
    //     perror("Error sending data");
    //     return;
    // }
    // printf("Sent %zd bytes: '%s'\n", sent, message);

    // usleep(10000);
    // // Receive the response

    // char temp_b[4096];
    // int bytes_received_2 = recv(server_socket, temp_b, sizeof(temp_b), 0);
    // if (bytes_received_2 < 0)
    // {
    //     perror("Error in receiving data from storage server");
    //     exit(1);
    // }
    // temp_b[bytes_received_2] = '\0';
    // printf("Received request from storage server: %s\n", temp_b);
}

void initialze_storage_server()
{

    nm_server_addr_len = sizeof(nm_server_addr);
    nm_server_socket = socket(AF_INET, SOCK_STREAM, 0);
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
    trie_root = create_trie_node();

    storage_server_count = 0;
    for (int i = 0; i < INITIAL_STORAGE_SERVERS; i++)
    {
        printf("Waiting for storage server connection\n");
        int accept_status = accept(nm_server_socket, (struct sockaddr *)&nm_server_addr, &nm_server_addr_len);
        if (accept_status < 0)
        {
            perror("Error in accepting connection from storage server");
            exit(1);
        }
        add_storage_server(accept_status);
        // close(accept_status);
    }
}

int Send_storage_server_details(char *file_path_to_find)
{
    StorageServer *found_server = find_storage_server(trie_root, file_path_to_find);
    if (found_server)
    {
        printf("IP: %s\nServer port: %d\nClient port: %d\n", found_server->ip, found_server->server_port, found_server->client_port);
        return found_server->storage_server_number;
    }
    else
    {
        printf("Error: File path not found.\n");
        return -1;
    }
}

int file_path(const char *path)
{
    printf("File path is : %s\n", path);
    // return Storage server details to client
    char temp_path[100];
    strcpy(temp_path, path);
    // temp_path[strlen(temp_path) - 1] = '\0'; /// removing \n from the end
    if (temp_path[strlen(temp_path) - 1] == '\n')
    {
        temp_path[strlen(temp_path) - 1] = '\0';
    }
    int details_of_storage_server = Send_storage_server_details(temp_path);
    printf("Details of storage server is : %d\n", details_of_storage_server);
    return details_of_storage_server;
}

// void print_all_accessible_paths(int accept_status_1)
// {
//     // print all accessible paths
//     printf("Printing all accessible paths\n");
//     printf("Number of storage servers are : %d\n", storage_server_count);
//     int total_paths = 0;
//     for (int i = 0; i < INITIAL_STORAGE_SERVERS; i++)
//     {
//         int number_of_paths = storage_servers[i].num_accessible_paths;
//         printf("Number of paths are : %d\n", number_of_paths);
//         for (int j = 0; j < number_of_paths; j++)
//         {
//             total_paths++;
//             printf("%s\n", storage_servers[i].accessible_paths[j]);
//         }
//     }
//     char temp_store[MAX_FILE_NAME_SIZE];

//     for (int i = 0; i < INITIAL_STORAGE_SERVERS; i++)
//     {
//         for (int j = 0; j < storage_servers[i].num_accessible_paths; j++)
//         {
//             strcat(temp_store, storage_servers[i].accessible_paths[j]);
//             if (i == INITIAL_STORAGE_SERVERS - 1 && j == storage_servers[i].num_accessible_paths - 1)
//             {   
//                 //
//             }
//             else
//             {
//                 strcat(temp_store, "\n");
//             }
//         }
//     }
//     usleep(1000);
//     send(accept_status_1, temp_store, sizeof(temp_store), 0);
// }
// Helper function to traverse trie and collect paths
void traverse_and_collect_paths(TrieNode* node, char* current_path, int depth, 
                              char** paths, int* path_count, int max_paths) {
    if (!node || *path_count >= max_paths) {
        return;
    }

    // If this is an end node with a server, save the path
    if (node->isEndOfPath && node->server) {
        current_path[depth] = '\0';  // Null terminate the current path
        paths[*path_count] = strdup(current_path);  // Make a copy of the path
        if (paths[*path_count]) {  // Check if strdup succeeded
            (*path_count)++;
        }
    }

    // Traverse all possible children
    for (int i = 0; i < 256; i++) {
        if (node->children[i]) {
            current_path[depth] = (char)i;
            traverse_and_collect_paths(node->children[i], current_path, 
                                    depth + 1, paths, path_count, max_paths);
        }
    }
}

// Modified print_all_accessible_paths function
void print_all_accessible_paths(TrieNode* root, int accept_status_1) {
    if (!root) {
        fprintf(stderr, "[ERROR] print_all_accessible_paths: Null trie root\n");
        return;
    }

    // Initialize variables for path collection
    const int MAX_PATHS = 1000;  // Adjust based on your needs
    const int MAX_PATH_LENGTH = 4096;  // Maximum path length
    char** paths = malloc(MAX_PATHS * sizeof(char*));
    char* current_path = malloc(MAX_PATH_LENGTH * sizeof(char));
    int path_count = 0;

    if (!paths || !current_path) {
        fprintf(stderr, "[ERROR] print_all_accessible_paths: Memory allocation failed\n");
        free(paths);
        free(current_path);
        return;
    }

    // Collect all paths from the trie
    traverse_and_collect_paths(root, current_path, 0, paths, &path_count, MAX_PATHS);

    // Print paths and prepare response
    printf("Printing all accessible paths\n");
    printf("Number of paths: %d\n", path_count);

    // Prepare response string
    char* temp_store = malloc(MAX_PATHS * MAX_PATH_LENGTH * sizeof(char));
    if (!temp_store) {
        fprintf(stderr, "[ERROR] print_all_accessible_paths: Memory allocation failed for temp_store\n");
        goto cleanup;
    }
    temp_store[0] = '\0';  // Initialize empty string

    // Build response string
    for (int i = 0; i < path_count; i++) {
        printf("%s\n", paths[i]);  // Print to console
        strcat(temp_store, paths[i]);
        if (i < path_count - 1) {
            strcat(temp_store, "\n");
        }
    }

    // Send response
    usleep(1000);
    send(accept_status_1, temp_store, strlen(temp_store) + 1, 0);

cleanup:
    // Clean up allocated memory
    for (int i = 0; i < path_count; i++) {
        free(paths[i]);
    }
    free(paths);
    free(current_path);
    free(temp_store);
}

void send_to_client(int accept_status_1, int ss_number)
{
    // char temp_ip[INET_ADDRSTRLEN];
    // strcpy(temp_ip, storage_servers[ss_number].ip);
    // send(accept_status_1, temp_ip, INET_ADDRSTRLEN, 0);
    // usleep(1000);
    int temp_port = storage_servers[ss_number].client_port;
    char temp_port_1[10];
    sprintf(temp_port_1, "%d", temp_port);
    // send(accept_status_1, &temp_port, sizeof(int), 0);

    char temp_store[1000];
    strcpy(temp_store, storage_servers[ss_number].ip);
    strcat(temp_store, " ");
    // strcat(temp_store, temp_port);
    strcat(temp_store, temp_port_1);
    printf("Sent storage server details to client : %s\n", temp_store);
    send(accept_status_1, temp_store, strlen(temp_store), 0);
}

void send_to_storage_server(int ss_number, char *file_path, char *name)
{
    send(storage_servers[ss_number].server_port, file_path, strlen(file_path), 0);
    usleep(1000);
    send(storage_servers[ss_number].server_port, name, strlen(name), 0);
}

void connect_and_send_SS(int SS_number, char *send_message, int accept_status_1)
{

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        perror("Error in creating server socket");
        exit(1);
    }
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(storage_servers[SS_number].server_port);

    if (inet_pton(AF_INET, storage_servers[SS_number].ip, &server_addr.sin_addr) <= 0)
    {
        perror("Invalid address/ Address not supported");
        close(server_socket);
        return;
    }

    int con_e = connect(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (con_e < 0)
    {
        perror("Error in connecting to storage server");
        exit(1);
    }
    printf("SENDING MESSAGE %s \n", send_message);

    send(server_socket, send_message, strlen(send_message), 0);

    char temp_b[4096];
    int bytes_received_2 = recv(server_socket, temp_b, sizeof(temp_b), 0);
    if (bytes_received_2 < 0)
    {
        perror("Error in receiving data from storage server");
        exit(1);
    }
    temp_b[bytes_received_2] = '\0';
    printf("Received request from storage server: %s\n", temp_b);
    send(accept_status_1, temp_b, strlen(temp_b), 0);
    // close(server_socket);
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

    int open_status = open("Log.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (open_status < 0)
    {
        perror("Error in opening file");
        exit(1);
    }

    write(open_status, "Received request from client: ", strlen("Received request from client: "));
    write(open_status, buffer, strlen(buffer));
    write(open_status, "\n", strlen("\n"));

    // const char *response = "ACK: Request received\n";
    // send(accept_status_1, response, strlen(response), 0);
    if (strncmp(buffer, "READ", 4) == 0)
    {
        int ss_number = file_path(buffer + 5);
        if (ss_number != -1)
        {
            send_to_client(accept_status_1, ss_number);
        }
        else
        {
            send(accept_status_1, "Path not found\n", strlen("Path not found\n"), 0);
        }
    }
    else if (strncmp(buffer, "LIST", 4) == 0)
    {
        print_all_accessible_paths(trie_root, accept_status_1);
    }
    else if (strncmp(buffer, "WRITE", 5) == 0) /// need to handle concurrent write requests from multiple clients to same file
    {
        int ss_number = file_path(buffer + 6);
        if (ss_number != -1)
        {
            send_to_client(accept_status_1, ss_number);
        }
        else
        {
            send(accept_status_1, "Path not found\n", strlen("Path not found\n"), 0);
        }
    }
    else if (strncmp(buffer, "GET_INFO", 8) == 0)
    {
        int ss_number = file_path(buffer + 9);
        if (ss_number != -1)
        {
            send_to_client(accept_status_1, ss_number);
        }
        else
        {
            send(accept_status_1, "Path not found\n", strlen("Path not found\n"), 0);
        }
    }
    else if (strncmp(buffer, "STREAM", 6) == 0)
    {

        int ss_number = file_path(buffer + 7);
        if (ss_number != -1)
        {
            send_to_client(accept_status_1, ss_number);
        }
        else
        {
            send(accept_status_1, "Path not found\n", strlen("Path not found\n"), 0);
        }
    }
    else if (strncmp(buffer, "DELETE", 6) == 0) //// i have to remove it from the cache also
    {
        int ss_number = file_path(buffer + 7);
        delete_path(trie_root, buffer + 7);
        if (ss_number != -1)
        {
            // send_to_client(accept_status_1, ss_number);
            // send(storage_servers[ss_number].file_descriptor, buffer, strlen(buffer), 0);
            // char temp_reply[4096];
            // int bytes_received = recv(storage_servers[ss_number].file_descriptor, temp_reply, 50, 0);
            // // int bytes_received = recv(accept_status_1,temp_reply,sizeof(temp_reply)-1,0);
            // if (bytes_received < 0)
            // {
            //     perror("Error in receiving data from storage server");
            //     exit(1);
            // }
            // temp_reply[bytes_received] = '\0';
            // printf("Received request from storage server: %s\n", temp_reply);
            // char temp[] = "ack sent\n";
            // // send(accept_status_1, temp_reply, strlen(temp_reply), 0);
            // send(accept_status_1, temp, sizeof(temp) - 1, 0);
            // printf("SENT request to client\n");

            connect_and_send_SS(ss_number, buffer, accept_status_1);
        }
        else
        {
            send(accept_status_1, "Path not found\n", strlen("Path not found\n"), 0);
        }
    }
    else if (strncmp(buffer, "CREATE", 6) == 0)
    {
        char temp_store[1000];
        strcpy(temp_store, buffer + 7);
        char *tokens[100];
        tokenize(temp_store, " ", tokens);
        for (int i = 0; tokens[i] != NULL; i++)
        {
            printf("Token is : %s\n", tokens[i]);
        }
        int ss_number = file_path(tokens[1]);
        if (ss_number != -1)
        {
            char temp_path_store[2000];
            strcpy(temp_path_store, tokens[1]);
            strcat(temp_path_store, "/");
            strcat(temp_path_store, tokens[2]);
            printf("insert file is %s\n", temp_path_store);
            insert_path(trie_root, temp_path_store, &storage_servers[ss_number]); //// not happening properly

            // send(storage_servers[ss_number].file_descriptor, buffer, strlen(buffer), 0);
            // char temp_reply[4096];

            // int bytes_received = recv(storage_servers[ss_number].file_descriptor, temp_reply, 50, 0);
            // // int bytes_received = recv(accept_status_1,temp_reply,sizeof(temp_reply)-1,0);
            // if (bytes_received < 0)
            // {
            //     perror("Error in receiving data from storage server");
            //     exit(1);
            // }
            // temp_reply[bytes_received] = '\0';
            // printf("Received request from storage server: %s\n", temp_reply);
            // char temp[] = "ack sent\n";
            // // send(accept_status_1, temp_reply, strlen(temp_reply), 0);
            // send(accept_status_1, temp, sizeof(temp) - 1, 0);
            // printf("SENT request to client\n");
            connect_and_send_SS(ss_number, buffer, accept_status_1);
        }

        else
        {
            printf("Storage server not found\n");
            send(accept_status_1, "Path not found\n", strlen("Path not found\n"), 0);
        }
    }

    else if (strncmp(buffer, "COPY", 4) == 0) /// i have to update the cache also
    {
        char temp_store[1000];
        strcpy(temp_store, buffer + 5);
        char *tokens[100];
        tokenize(temp_store, " ", tokens);
        for (int i = 0; tokens[i] != NULL; i++)
        {
            printf("Token is : %s\n", tokens[i]);
        }
        int ss_number = file_path(tokens[0]);
        int ss_number_1 = file_path(tokens[1]);
        update_server(trie_root, tokens[1], &storage_servers[ss_number_1]);
        if (ss_number != -1 && ss_number_1 != -1)
        {
            // send_to_storage_server(ss_number, tokens[0], tokens[2]);
            // send_to_storage_server(ss_number_1, tokens[1], tokens[2]);

            char temp_command_1[1000];
            strcpy(temp_command_1, "READ ");
            strcat(temp_command_1, tokens[0]);
            int server_socket = socket(AF_INET, SOCK_STREAM, 0);
            if (server_socket < 0)
            {
                perror("Error in creating server socket");
                exit(1);
            }
            struct sockaddr_in server_addr;
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(storage_servers[ss_number].server_port);
            if (inet_pton(AF_INET, storage_servers[ss_number].ip, &server_addr.sin_addr) <= 0)
            {
                perror("Invalid address/ Address not supported");
                close(server_socket);
                // return;
            }
            int con_e = connect(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
            if (con_e < 0)
            {
                perror("Error in connecting to storage server");
                exit(1);
            }
            printf("SENDING MESSAGE %s \n", temp_command_1);

            send(server_socket, temp_command_1, strlen(temp_command_1), 0);

            packet packet_1;

            int rec_packet = recv(server_socket, &packet_1, sizeof(packet), 0);
            if (rec_packet < 0)
            {
                perror("Error in receiving data from storage server");
                exit(1);
            }
            // printf("Received request from storage server: %s\n", packet_1.data);
            packet pkt[packet_1.total_chunks - 1];

            for (int i = 0; i < packet_1.total_chunks - 1; i++)
            {
                int rec_packet = recv(server_socket, &pkt[i], sizeof(packet), 0);
                if (rec_packet < 0)
                {
                    perror("Error in receiving data from storage server");
                    exit(1);
                }
            }

            char temp_command_2[1000];
            strcpy(temp_command_2, "WRITE ");
            strcat(temp_command_2, tokens[1]);
            usleep(1000);

            send(server_socket, temp_command_2, strlen(temp_command_2), 0);

            
            packet temp_pkt;
            int recv_temp=recv(server_socket,&temp_pkt,sizeof(packet),0);
            if(recv_temp<0)
            {
                perror("Error in receiving data from storage server");
                exit(1);
            }
            recv_temp=recv(server_socket,&temp_pkt,sizeof(packet),0);
            if(recv_temp<0)
            {
                perror("Error in receiving data from storage server");
                exit(1);
            }
            printf("Received request from storage server: %s\n", temp_pkt.data);
            // printf("Received request from storage server: %s\n", etmp);
            printf("Packet 1 is : %s\n", packet_1.data);


            send(server_socket, packet_1.data, sizeof(packet), 0);
            for (int i = 0; i < packet_1.total_chunks - 1; i++)
            {
                printf("Packet %d is : %s\n", i, pkt[i].data);
                send(server_socket, pkt[i].data, sizeof(packet), 0);
            }
            temp_pkt.seq_num=-1;
            send(server_socket,&temp_pkt,sizeof(packet),0);
            char bufr[1000];
            recv(server_socket, bufr, sizeof(bufr) - 1, 0);
            printf("Received request from storage server: %s\n", bufr);
            send(accept_status_1, bufr, strlen(bufr), 0);

        }
        else
        {
            printf("Storage server not found\n");
            send(accept_status_1, "Path not found\n", strlen("Path not found\n"), 0);
        }
    }
    else if (strncmp(buffer, "SERVER", 6) == 0)
    {
        add_storage_server(accept_status_1);
    }
    else
    {
        printf("Invalid request\n");
        send(accept_status_1, "Invalid request\n", strlen("Invalid request\n"), 0);
    }
    return NULL;
    close(open_status);
    close(accept_status_1);
}

// // Global cache instance
// LRUCache *file_location_cache = NULL;

// // Initialize cache at program startup
// void init_file_location_cache()
// {
//     cache_error_t error;
//     file_location_cache = init_cache(1000, &error); // Adjust capacity as needed
//     if (!file_location_cache)
//     {
//         fprintf(stderr, "Failed to initialize cache: %s\n", cache_error_string(error));
//         exit(1);
//     }
// }

// Helper function to handle cache operations for file lookups
// int get_storage_server_with_cache(const char *filepath)
// {
//     if (!filepath)
//         return -1;

//     // Try to get from cache first
//     int ss_number = cache_get(file_location_cache, filepath);
//     if (ss_number != -1)
//     {
//         return ss_number;
//     }

//     // Cache miss - look up in trie
//     ss_number = file_path(filepath);
//     if (ss_number != -1)
//     {
//         // Add to cache for future lookups
//         cache_put(file_location_cache, filepath, ss_number);
//     }

//     return ss_number;
// }

// void *process_client_requests(void *accept_status)
// {
//     int accept_status_1 = *(int *)accept_status;
//     char buffer[BUFFER_SIZE];
//     int bytes_received = recv(accept_status_1, buffer, sizeof(buffer) - 1, 0);
//     if (bytes_received < 0)
//     {
//         perror("Error in receiving data from client");
//         exit(1);
//     }
//     buffer[bytes_received] = '\0';
//     printf("Received request from client: %s\n", buffer);

//     // Log request
//     int open_status = open("Log.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);
//     if (open_status < 0)
//     {
//         perror("Error in opening file");
//         exit(1);
//     }
//     write(open_status, "Received request from client: ", strlen("Received request from client: "));
//     write(open_status, buffer, strlen(buffer));
//     write(open_status, "\n", strlen("\n"));
//     close(open_status);

//     const char *response = "ACK: Request received\n";
//     send(accept_status_1, response, strlen(response), 0);

//     if (strncmp(buffer, "READ", 4) == 0)
//     {
//         int ss_number = get_storage_server_with_cache(buffer + 5);
//         if (ss_number != -1)
//         {
//             send_to_client(accept_status_1, ss_number);
//         }
//     }
//     else if (strncmp(buffer, "WRITE", 5) == 0)
//     {
//         int ss_number = get_storage_server_with_cache(buffer + 6);
//         if (ss_number != -1)
//         {
//             send_to_client(accept_status_1, ss_number);
//         }
//     }
//     else if (strncmp(buffer, "DELETE", 6) == 0)
//     {
//         const char *filepath = buffer + 7;
//         int ss_number = get_storage_server_with_cache(filepath);
//         if (ss_number != -1)
//         {
//             // Remove from both trie and cache
//             delete_path(trie_root, filepath);
//             hashmap_remove(file_location_cache->map, filepath);
//             send_to_client(accept_status_1, ss_number);
//         }
//     }
//     else if (strncmp(buffer, "CREATE", 6) == 0)
//     {
//         char temp_store[1000];
//         strcpy(temp_store, buffer + 7);
//         char *tokens[100];
//         tokenize(temp_store, " ", tokens);

//         int ss_number = get_storage_server_with_cache(tokens[0]);
//         // Add to both trie and cache
//         insert_path(trie_root, tokens[0], &storage_servers[ss_number]);
//         cache_put(file_location_cache, tokens[0], ss_number);

//         if (ss_number != -1)
//         {
//             send_to_storage_server(ss_number, tokens[0], tokens[1]);
//         }
//     }
//     else if (strncmp(buffer, "COPY", 4) == 0)
//     {
//         char temp_store[1000];
//         strcpy(temp_store, buffer + 5);
//         char *tokens[100];
//         tokenize(temp_store, " ", tokens);

//         int ss_number = get_storage_server_with_cache(tokens[0]);
//         int ss_number_1 = get_storage_server_with_cache(tokens[1]);

//         // Update both trie and cache
//         update_server(trie_root, tokens[0], &storage_servers[ss_number_1]);
//         cache_put(file_location_cache, tokens[0], ss_number_1);

//         if (ss_number != -1 && ss_number_1 != -1)
//         {
//             send_to_storage_server(ss_number, tokens[0], tokens[2]);
//             send_to_storage_server(ss_number_1, tokens[1], tokens[2]);
//         }
//     }

//     // Other cases remain the same...
//     return NULL;
// }

// // Clean up cache when shutting down
// void cleanup_cache()
// {
//     if (file_location_cache)
//     {
//         print_cache_stats(file_location_cache); // Print final statistics
//         free_cache(file_location_cache);
//         file_location_cache = NULL;
//     }
// }

void *Handle_client_requests(void *arg)
{
    // struct sockaddr_in client_addr;
    // socklen_t client_addr_len = sizeof(client_addr);
    // int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    // if (client_socket < 0)
    // {
    //     perror("Error in creating client socket");
    //     exit(1);
    // }
    // client_addr.sin_family = AF_INET;
    // client_addr.sin_port = htons(CLIENT_PORT);
    // client_addr.sin_addr.s_addr = INADDR_ANY;
    // if (bind(client_socket, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0)
    // {
    //     perror("Error in binding client socket");
    //     exit(1);
    // }
    // int listen_status = listen(client_socket, MAX_CLIENTS);
    // if (listen_status < 0)
    // {
    //     perror("Error in listening to client socket");
    //     exit(1);
    // }
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    while (1)
    {
        printf("Waiting for client connection\n");
        int accept_status = accept(nm_server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (accept_status < 0)
        {
            perror("Error in accepting connection from client");
            exit(1);
        }
        pthread_t thread;
        pthread_create(&thread, NULL, process_client_requests, &accept_status);
        // pthread_detach(thread);
    }
}

void free_all()
{
    free(trie_root);
}

int main()
{
    // init_file_location_cache();
    initialze_storage_server();
    pthread_t client_thread;
    pthread_create(&client_thread, NULL, Handle_client_requests, NULL);
    pthread_join(client_thread, NULL);
    // cleanup_cache();
    free_all();
    return 0;
}