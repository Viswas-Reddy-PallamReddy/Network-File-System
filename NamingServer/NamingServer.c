#include "NamingServer.h"

struct sockaddr_in nm_server_addr;
socklen_t nm_server_addr_len;
int nm_server_socket;
LRUCache *file_location_cache = NULL;
StorageServer storage_servers[MAX_STORAGE_SERVERS];
TrieNode *trie_root = NULL;
int storage_server_count = 0;

int clients_File_Descriptors[MAX_CLIENTS];
int client_count = 0;


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

void write_log_2(StorageServer new_storage_server)
{
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
    close(open_status);
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
    int te_port = atoi(temp_3[1]);
    for(int i=0;i<storage_server_count;i++)
    {
         if (storage_servers[i].server_port == te_port && strcmp(storage_servers[i].ip, temp_3[0]) == 0)
        {
            printf("\033[0;32mStorage server coming back online\n\033[0m");
            storage_servers[i].server_down = false;
            /// need to call a function
        }
    }


    strcpy(new_storage_server.ip, temp_3[0]);
    new_storage_server.server_port = atoi(temp_3[1]);
    new_storage_server.client_port = atoi(temp_3[2]);
    new_storage_server.num_accessible_paths = atoi(temp_3[3]);

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
    printf("PAths receviing are : %s\n", temp);

    char *temp_4[MAX_FILE_NAME_SIZE];
    tokenize(temp, "\n", temp_4);
    // printf("Tokenized\n");

    for (int i = 0; i < new_storage_server.num_accessible_paths; i++)
    {
        // printf("Token %d is : %s\n", i, temp_4[i]);
        strcpy(new_storage_server.accessible_paths[i], temp_4[i]);
        // printf("Path is : %s\n", new_storage_server.accessible_paths[i]);
    }
    // if (storage_server_count == 0 || storage_server_count == 1)
    // {
    //     new_storage_server.Backup_SS = true;
    // }
    // else
    // {
    //     new_storage_server.Backup_SS = false;
    // }

    storage_servers[storage_server_count++] = new_storage_server;

    // for (int i = 0; i < new_storage_server.num_accessible_paths; i++)
    // {
    //     printf("Registered storage server accessible paths : %s\n", new_storage_server.accessible_paths[i]);
    // }

    // Add the accessible paths to the trie
    for (int i = 0; i < new_storage_server.num_accessible_paths; i++)
    {
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
        close(open_status);
        insert_path(trie_root, new_storage_server.accessible_paths[i], &storage_servers[storage_server_count - 1]);
    }

    write_log_2(new_storage_server);
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
    int listen_status = listen(nm_server_socket, MAX_CLIENTS);
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

        char temp_s[10];
        int bytes_received_s = recv(accept_status, temp_s, sizeof(temp_s), 0);
        if (bytes_received_s < 0)
        {
            perror("Error in receiving data from storage server");
            exit(1);
        }
        printf("TEMP_S is : %s\n", temp_s);
        if (strncmp(temp_s, "SERVER", 6) != 0)
        {
            printf("Invalid request_By storage server\n");
            // send(accept_status, "Invalid request\n", strlen("Invalid request\n"), 0);
            return;
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

void traverse_and_collect_paths(TrieNode *node, char *current_path, int depth,
                                char **paths, int *path_count, int max_paths)
{
    if (!node || *path_count >= max_paths)
    {
        return;
    }

    // If this is an end node with a server, save the path
    if (node->isEndOfPath && node->server)
    {
        current_path[depth] = '\0';                // Null terminate the current path
        paths[*path_count] = strdup(current_path); // Make a copy of the path
        if (paths[*path_count])
        { // Check if strdup succeeded
            (*path_count)++;
        }
    }

    // Traverse all possible children
    for (int i = 0; i < 256; i++)
    {
        if (node->children[i])
        {
            current_path[depth] = (char)i;
            traverse_and_collect_paths(node->children[i], current_path,
                                       depth + 1, paths, path_count, max_paths);
        }
    }
}

// Modified print_all_accessible_paths function
void print_all_accessible_paths(TrieNode *root, int accept_status_1)
{
    if (!root)
    {
        fprintf(stderr, "[ERROR] print_all_accessible_paths: Null trie root\n");
        return;
    }

    // Initialize variables for path collection
    const int MAX_PATHS = 1000;       // Adjust based on your needs
    const int MAX_PATH_LENGTH = 4096; // Maximum path length
    char **paths = malloc(MAX_PATHS * sizeof(char *));
    char *current_path = malloc(MAX_PATH_LENGTH * sizeof(char));
    int path_count = 0;

    if (!paths || !current_path)
    {
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
    char *temp_store = malloc(MAX_PATHS * MAX_PATH_LENGTH * sizeof(char));
    if (!temp_store)
    {
        fprintf(stderr, "[ERROR] print_all_accessible_paths: Memory allocation failed for temp_store\n");
        goto cleanup;
    }
    temp_store[0] = '\0'; // Initialize empty string

    // Build response string
    for (int i = 0; i < path_count; i++)
    {
        printf("%s\n", paths[i]); // Print to console
        strcat(temp_store, paths[i]);
        if (i < path_count - 1)
        {
            strcat(temp_store, "\n");
        }
    }

    // Send response
    usleep(1000);
    send(accept_status_1, temp_store, strlen(temp_store) + 1, 0);

cleanup:
    // Clean up allocated memory
    for (int i = 0; i < path_count; i++)
    {
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
    strcpy(temp_store, "IP ");
    strcat(temp_store, storage_servers[ss_number].ip);
    strcat(temp_store, " ");
    strcat(temp_store, "PORT ");
    // strcat(temp_store, temp_port);
    strcat(temp_store, temp_port_1);
    printf("Sent storage server details to client : %s\n", temp_store);
    send(accept_status_1, temp_store, strlen(temp_store), 0);
}

void send_to_storage_server(int ss_number, char *file_path, char *name)
{
    send(storage_servers[ss_number].server_port, file_path, strlen(file_path), 0);
    usleep(5000);
    send(storage_servers[ss_number].server_port, name, strlen(name), 0);
}

int get_storage_server_with_cache(const char *filepath)
{
    if (!filepath)
        return -1;

    // Try to get from cache first
    int ss_number = cache_get(file_location_cache, filepath);
    if (ss_number != -1)
    {
        return ss_number;
    }

    // Cache miss - look up in trie
    ss_number = file_path(filepath);
    if (ss_number != -1)
    {
        // Add to cache for future lookups
        cache_put(file_location_cache, filepath, ss_number);
    }

    return ss_number;
}

struct FilePath
{
    char path[MAX_PATH_LENGTH_1]; // Assuming MAX_PATH_LENGTH is defined
    struct FilePath *next;
};

struct FilePath *writing_files_list = NULL;

// Function to check if a file path is already in the writing list
int is_file_path_being_written(const char *path)
{
    struct FilePath *current = writing_files_list;
    while (current != NULL)
    {
        if (strcmp(current->path, path) == 0)
        {
            return 1; // Path is already being written
        }
        current = current->next;
    }
    return 0; // Path is not in the list
}

// Function to add a file path to the writing list
void add_file_path_to_writing_list(const char *path)
{
    struct FilePath *new_path = malloc(sizeof(struct FilePath));
    if (new_path == NULL)
    {
        // Handle memory allocation error
        return;
    }
    strncpy(new_path->path, path, MAX_PATH_LENGTH_1 - 1);
    new_path->path[MAX_PATH_LENGTH_1 - 1] = '\0'; // Ensure null-termination

    new_path->next = writing_files_list;
    writing_files_list = new_path;
}

// Function to remove a file path from the writing list
void remove_file_path_from_writing_list(const char *path)
{
    struct FilePath *current = writing_files_list;
    struct FilePath *prev = NULL;

    while (current != NULL)
    {
        if (strcmp(current->path, path) == 0)
        {
            if (prev == NULL)
            {
                writing_files_list = current->next;
            }
            else
            {
                prev->next = current->next;
            }
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}

// Helper function to check if a path is a prefix of another path
static int is_prefix(const char *prefix, const char *path)
{
    size_t prefix_len = strlen(prefix);
    if (strlen(path) < prefix_len)
        return 0;

    // Check if prefix matches and if the next character is '/' or end of string
    return (strncmp(prefix, path, prefix_len) == 0 &&
            (path[prefix_len] == '/' || path[prefix_len] == '\0'));
}

// Helper function to collect all paths with a given prefix
static void collect_paths_with_prefix(TrieNode *root, char *current_path, int current_len,
                                      const char *prefix, char **paths, int *path_count)
{
    if (!root)
        return;

    // If we found a complete path, check if it matches our prefix
    if (root->isEndOfPath)
    {
        current_path[current_len] = '\0';
        if (is_prefix(prefix, current_path))
        {
            paths[*path_count] = strdup(current_path);
            (*path_count)++;
        }
    }

    // Recursively check all children
    for (int i = 0; i < 256; i++)
    {
        if (root->children[i])
        {
            current_path[current_len] = (char)i;
            collect_paths_with_prefix(root->children[i], current_path, current_len + 1,
                                      prefix, paths, path_count);
        }
    }
}

// Modified delete handler
TrieError delete_path_recursive(TrieNode *root, const char *base_path,
                                HashMap *cache)
{
    if (!root || !base_path)
    {
        return TRIE_NULL_POINTER;
    }
    // int ss_number = get_storage_server_with_cache(base_path);
    int ss_number = file_path(base_path);
    if (ss_number == -1)
    {
        return TRIE_PATH_NOT_FOUND;
    }

    // First collect all paths that start with base_path
    char **paths = malloc(sizeof(char *) * 4096); // Assume max 4096 paths
    int path_count = 0;
    char *current_path = malloc(4096); // Assume max path length of 4096

    // Collect all paths that start with base_path
    collect_paths_with_prefix(root, current_path, 0, base_path, paths, &path_count);

    // Sort paths in reverse order (longer paths first) to ensure children are deleted before parents
    for (int i = 0; i < path_count; i++)
    {
        for (int j = i + 1; j < path_count; j++)
        {
            if (strlen(paths[j]) > strlen(paths[i]))
            {
                char *temp = paths[i];
                paths[i] = paths[j];
                paths[j] = temp;
            }
        }
    }

    // Delete each path
    for (int i = 0; i < path_count; i++)
    {
        // Delete from trie
        delete_path(root, paths[i]);

        // Delete from cache
        hashmap_remove(cache, paths[i]);

        // Send delete command to storage server

        free(paths[i]);
    }

    // Finally, delete the base path itself if it exists
    delete_path(root, base_path);
    hashmap_remove(cache, base_path);

    free(paths);
    free(current_path);

    return TRIE_SUCCESS;
}

void write_log(char *temp_b)
{
    int open_status = open("Log.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (open_status < 0)
    {
        perror("Error in opening file");
        exit(1);
    }
    write(open_status, "Received ACKNOWLEDGEMENT from storage server: ", strlen("Received request from storage server: "));
    write(open_status, temp_b, strlen(temp_b));
    write(open_status, "\n", strlen("\n"));
    close(open_status);
}

int connect_and_send_SS(int SS_number, char *send_message, int accept_status_1)
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
        return -1;
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

    write_log(temp_b);

    temp_b[bytes_received_2] = '\0';

    printf("Received request from storage server: %s\n", temp_b);
    send(accept_status_1, temp_b, strlen(temp_b), 0);
    if (strcmp(temp_b, "ERROR 0") == 0)
    {
        printf("SENT request to client\n");
        return 1;
    }
    else if (strcmp(temp_b, "ERROR 6") == 0)
    {
        printf("ERROR 6 from server\n");
        return 0;
    }
    else if (strcmp(temp_b, "ERROR 7") == 0)
    {
        printf("ERROR 7 from server\n");
        return 0;
    }
    else if (strcmp(temp_b, "ERROR 5") == 0)
    {
        printf("ERROR 5 from server\n");
        return 0;
    }
    else
    {
        printf("ERROR from server_Else wala\n");
        return 0;
    }
    // close(server_socket);
}
void *process_client_requests(void *accept_status)
{
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

    // Log request
    int open_status = open("Log.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (open_status < 0)
    {
        perror("Error in opening file");
        exit(1);
    }
    write(open_status, "Received request from client: ", strlen("Received request from client: "));
    write(open_status, buffer, strlen(buffer));
    write(open_status, "\n", strlen("\n"));
    close(open_status);

    if (strncmp(buffer, "READ", 4) == 0)
    {
        char *file_path = buffer + 5;

        if (is_file_path_being_written(file_path))
        {
            send(accept_status_1, "ERROR 4\n", strlen("ERROR 4\n"), 0);
            return NULL;
        }

        int ss_number = get_storage_server_with_cache(buffer + 5);
        if (ss_number != -1)
        {
            send_to_client(accept_status_1, ss_number);
        }
        else
        {
            send(accept_status_1, "ERROR 2\n", strlen("ERROR 2\n"), 0);
        }
    }
    else if (strncmp(buffer, "WRITE", 5) == 0)
    {
        char *file_path = buffer + 6;

        if (is_file_path_being_written(file_path))
        {
            send(accept_status_1, "ERROR 4\n", strlen("ERROR 4\n"), 0);
            return NULL;
        }

        add_file_path_to_writing_list(file_path);

        int ss_number = get_storage_server_with_cache(buffer + 6);
        if (ss_number != -1)
        {
            send_to_client(accept_status_1, ss_number);
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
                return NULL;
            }
            int con_e = connect(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
            if (con_e < 0)
            {
                perror("Error in connecting to storage server");
                exit(1);
            }
            printf("The Connection is established\n");
            char temp_command[1000];
            int bytes_received = recv(server_socket, temp_command, sizeof(temp_command), 0);
            if (bytes_received < 0)
            {
                perror("Error in receiving data from storage server");
                exit(1);
            }
            write_log(temp_command);

            temp_command[bytes_received] = '\0';
            printf("Received request from storage server: %s\n", temp_command);
            remove_file_path_from_writing_list(file_path);
        }
        else
        {
            send(accept_status_1, "ERROR 2\n", strlen("ERROR 2\n"), 0);
        }
    }
    else if (strncmp(buffer, "DELETE", 6) == 0)
    {

        int ss_number = file_path(buffer + 7);
        if (ss_number != -1)
        {
            int status = connect_and_send_SS(ss_number, buffer, accept_status_1);
            if (status == 1)
            {

                const char *filepath = buffer + 7;
                TrieError result = delete_path_recursive(trie_root, filepath,
                                                         file_location_cache->map);
                if (result != TRIE_SUCCESS)
                {
                    send(accept_status_1, "Path not found_BY\n", strlen("Path not found_BY\n"), 0);
                }
            }
            else if (status == 0)
            {
                send(accept_status_1, "Path not found_BYE\n", strlen("Path not found_BYE\n"), 0);
            }
        }
        else
        {
            send(accept_status_1, "ERROR 2\n", strlen("ERROR 2\n"), 0);
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

        int ss_number = get_storage_server_with_cache(tokens[1]);

        if (ss_number != -1)
        {
            char temp_path_store[2000];
            strcpy(temp_path_store, tokens[1]);
            strcat(temp_path_store, "/");
            strcat(temp_path_store, tokens[2]);
            printf("insert file is %s\n", temp_path_store);
            StorageServer *temp_r = find_storage_server(trie_root, temp_path_store);

            if (temp_r != NULL)
            {
                printf("File already exists\n");
                send(accept_status_1, "ERROR 5\n", strlen("ERROR 5\n"), 0);
                return NULL;
            }

            insert_path(trie_root, temp_path_store, &storage_servers[ss_number]); //// not happening properly
            // cache_put(file_location_cache, tokens[1], ss_number);
            cache_put(file_location_cache, temp_path_store, ss_number);

            connect_and_send_SS(ss_number, buffer, accept_status_1);
        }
        else
        {
            printf("Storage server not found\n");
            send(accept_status_1, "ERROR 2\n", strlen("ERROR 2\n"), 0);
        }
    }
    else if (strncmp(buffer, "COPY", 4) == 0)
    {
        char temp_store[1000];
        strcpy(temp_store, buffer + 5);
        char *tokens[100];
        tokenize(temp_store, " ", tokens);
        for (int i = 0; tokens[i] != NULL; i++)
        {
            printf("Token is : %s\n", tokens[i]);
        }

        int ss_number = get_storage_server_with_cache(tokens[0]);
        int ss_number_1 = get_storage_server_with_cache(tokens[1]);

        // Update both trie and cache

        if (ss_number != -1 && ss_number_1 != -1)
        {
            update_server(trie_root, tokens[1], &storage_servers[ss_number_1]);
            cache_put(file_location_cache, tokens[0], ss_number);
            cache_put(file_location_cache, tokens[1], ss_number_1);
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
            int recv_temp = recv(server_socket, &temp_pkt, sizeof(packet), 0);
            if (recv_temp < 0)
            {
                perror("Error in receiving data from storage server");
                exit(1);
            }
            recv_temp = recv(server_socket, &temp_pkt, sizeof(packet), 0);
            if (recv_temp < 0)
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
            temp_pkt.seq_num = -1;
            send(server_socket, &temp_pkt, sizeof(packet), 0);
            char bufr[1000];
            recv(server_socket, bufr, sizeof(bufr) - 1, 0);
            printf("Received request from storage server: %s\n", bufr);
            send(accept_status_1, bufr, strlen(bufr), 0);
        }
        else
        {
            printf("Storage server not found\n");
            send(accept_status_1, "ERROR 2\n", strlen("ERROR 2\n"), 0);
        }
    }
    else if (strncmp(buffer, "LIST", 4) == 0)
    {
        print_all_accessible_paths(trie_root, accept_status_1);
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
            send(accept_status_1, "ERROR 2\n", strlen("ERROR 2\n"), 0);
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
            send(accept_status_1, "ERROR 2\n", strlen("ERROR 2\n"), 0);
        }
    }
    else if (strncmp(buffer, "SERVER", 6) == 0)
    {
        add_storage_server(accept_status_1);
    }
    else
    {
        printf("Invalid request\n");
        send(accept_status_1, "ERROR 1\n", strlen("ERROR 1\n"), 0);
    }
    return NULL;
    // close(accept_status_1); //// need to think about this
}

// Clean up cache when shutting down
void cleanup_cache()
{
    if (file_location_cache)
    {
        print_cache_stats(file_location_cache); // Print final statistics
        free_cache(file_location_cache);
        file_location_cache = NULL;
    }
}
void *Handle_client_requests(void *arg)
{

    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    while (1)
    {
        printf("Waiting for client connection\n");
        clients_File_Descriptors[client_count++] = accept(nm_server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (clients_File_Descriptors[client_count-1] < 0)
        {
            perror("Error in accepting connection from client");
            exit(1);
        }
        pthread_t thread;
        pthread_create(&thread, NULL, process_client_requests, &clients_File_Descriptors[client_count-1]);
        // pthread_detach(thread);
    }
}

void free_all()
{
    free(trie_root);
}

// Initialize cache at program startup
void init_file_location_cache()
{
    cache_error_t error;
    file_location_cache = init_cache(INITIAL_CASHE_SIZE, &error); // Adjust capacity as needed
    if (!file_location_cache)
    {
        fprintf(stderr, "Failed to initialize cache: %s\n", cache_error_string(error));
        exit(1);
    }
}

void sigchld_handler(int sig)
{
    printf("Received SIGTSTP signal\n");
    FILE *file_op = fopen("Log.txt", "r");
    if (file_op == NULL)
    {
        perror("Error in opening file");
        exit(1);
    }
    // Move the file pointer to the beginning of the file
    fseek(file_op, 0, SEEK_SET);

    // Read and print the entire file
    char ch;
    while ((ch = fgetc(file_op)) != EOF)
    {
        putchar(ch);
    }

    // Close the file
    fclose(file_op);
}

void init_log_print()
{
    struct sigaction sa;
    sa.sa_handler = &sigchld_handler;
    sa.sa_flags = SA_RESTART;
    sigaction(SIGTSTP, &sa, NULL);
}

int ping_server(const char *ip, int port)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        return 0; // Socket creation failed
    }
    // Set up server address
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &server_addr.sin_addr);
    int send_it = 0;

    // Try to connect
    int result = connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (result < 0)
    {
        storage_servers[storage_server_count].server_down = true;
        perror("Error in connecting to storage server");
        printf("Server %s:%d is down\n", ip, port);
    }
    else
    {
        send_it = send(sock, "EXIT", strlen("EXIT"), 0);
        if (send_it < 0)
        {
            storage_servers[storage_server_count].server_down = true;
            perror("Error in sending data to storage server");
            printf("Server %s:%d is down\n", ip, port);
        }
    }
    close(sock);
    result = (result >= 0 && send_it >= 0);
    return result; // Return 1 if connected, 0 if failed
}

void *Server_Status(void *arg)
{

    while (1)
    {
        for (int i = 0; i < storage_server_count; i++)
        {
            // printf("Checking status of storage server %d\n", i);

            if (storage_servers[i].server_down==false && ping_server(storage_servers[i].ip, storage_servers[i].server_port)==0)
            {
                printf("Storage server %d is down\n", i);
                storage_servers[i].server_down = true;
                // Remove the server from the trie
                // remove_storage_server(trie_root, i);
            }
        }
        sleep(5);
    }
}

int main()
{
    init_log_print();
    init_file_location_cache();
    initialze_storage_server();
    pthread_t server_active_thread;
    pthread_create(&server_active_thread, NULL, Server_Status, NULL);
    pthread_t client_thread;
    pthread_create(&client_thread, NULL, Handle_client_requests, NULL);
    pthread_join(client_thread, NULL);
    cleanup_cache();
    free_all();
    return 0;
}