#include "NamingServer.h"

StorageServer storage_servers[MAX_STORAGE_SERVERS];
TrieNode *trie_root = NULL;
int storage_server_count = 0;

void add_storage_server(int accept_status)
{
    printf("Adding storage server\n");
    StorageServer new_storage_server;
    // int bytes_1 = recv(accept_status, new_storage_server.ip, INET_ADDRSTRLEN, 0);
    // if (bytes_1 < 0)
    // {
    //     perror("Error in receiving data from storage server");
    //     exit(1);
    // }
    // int bytes_2 = recv(accept_status, &new_storage_server.server_port, sizeof(int), 0);
    // if (bytes_2 < 0)
    // {
    //     perror("Error in receiving data from storage server");
    //     exit(1);
    // }
    // int bytes_3 = recv(accept_status, &new_storage_server.client_port, sizeof(int), 0);
    // if (bytes_3 < 0)
    // {
    //     perror("Error in receiving data from storage server");
    //     exit(1);
    // }
    // int bytes_4 = recv(accept_status, &new_storage_server.num_accessible_paths, sizeof(int), 0);
    // if (bytes_4 < 0)
    // {
    //     perror("Error in receiving data from storage server");
    //     exit(1);
    // }
    // for (int i = 0; i < new_storage_server.num_accessible_paths; i++)
    // {
    //     int path_lenth = 0;
    //     int bytes_5 = recv(accept_status, &path_lenth, sizeof(int), 0);
    //     if (bytes_5 < 0)
    //     {
    //         perror("Error in receiving data from storage server");
    //         exit(1);
    //     }
    //     printf("Path length is : %d\n", path_lenth);
    //     char path[path_lenth];
    //     int bytes_6 = recv(accept_status, path, path_lenth, 0);
    //     if (bytes_6 < 0)
    //     {
    //         perror("Error in receiving data from storage server");
    //         exit(1);
    //     }
    //     // printf("Path is : %s\n",path);
    //     new_storage_server.accessible_paths[i] = (char *)malloc(path_lenth);
    //     strcpy(new_storage_server.accessible_paths[i], path);
    // }
    int bytes_received = recv(accept_status, &new_storage_server, sizeof(StorageServer), 0);
    if(bytes_received < 0)
    {
        perror("Error in receiving data from storage server");
        exit(1);
    }

    printf("Received storage server details\n");
    printf("Storage server count is : %d\n", storage_server_count);
    new_storage_server.storage_server_number = storage_server_count;
    storage_servers[storage_server_count++] = new_storage_server;

    // // Add the accessible paths to the trie
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

    printf("Registered new storage server: %s\n", new_storage_server.ip);
    printf("Registered storage server Port for client : %d\n", new_storage_server.client_port);
    printf("Registered storage server Port for NM : %d\n", new_storage_server.server_port);
    printf("Registered storage server accessible paths : %d\n", new_storage_server.num_accessible_paths);
    printf("Storage server count is : %d\n", new_storage_server.storage_server_number);

    for (int i = 0; i < new_storage_server.num_accessible_paths; i++)
    {
        printf("Registered storage server accessible paths : %s\n", new_storage_server.accessible_paths[i]);
    }
    // printf("Registered storage server accessible paths : %s\n", new_storage_server.accessible_paths);
}

void initialze_storage_server()
{
    struct sockaddr_in nm_server_addr;
    socklen_t nm_server_addr_len = sizeof(nm_server_addr);
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

int file_path(char *path)
{
    printf("File path is : %s\n", path);
    // return Storage server details to client
    char temp_path[100];
    strcpy(temp_path, path);
    temp_path[strlen(temp_path) - 1] = '\0'; /// removing \n from the end
    int details_of_storage_server = Send_storage_server_details(temp_path);
    printf("Details of storage server is : %d\n", details_of_storage_server);
    return details_of_storage_server;
}

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

void print_all_accessible_paths()
{
    // print all accessible paths
    printf("Printing all accessible paths\n");
    printf("Number of storage servers are : %d\n", storage_server_count);
    for (int i = 0; i < INITIAL_STORAGE_SERVERS; i++)
    {
        int number_of_paths = storage_servers[i].num_accessible_paths;
        printf("Number of paths are : %d\n", number_of_paths);
        for (int j = 0; j < number_of_paths; j++)
        {
            printf("%s\n", storage_servers[i].accessible_paths[j]);
        }
    }
}

void send_to_client(int accept_status_1, int ss_number)
{
    char temp_ip[INET_ADDRSTRLEN];
    strcpy(temp_ip, storage_servers[ss_number].ip);
    send(accept_status_1, temp_ip, INET_ADDRSTRLEN, 0);
    usleep(1000);
    int temp_port = storage_servers[ss_number].client_port;
    send(accept_status_1, &temp_port, sizeof(int), 0);
}

void send_to_storage_server(int ss_number, char *file_path , char * name)
{
    send(storage_servers[ss_number].server_port, file_path, strlen(file_path), 0);
    usleep(1000);
    send(storage_servers[ss_number].server_port, name, strlen(name), 0);
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

    const char *response = "ACK: Request received\n";
    send(accept_status_1, response, strlen(response), 0);
    if (strncmp(buffer, "READ", 4) == 0)
    {
        int ss_number = file_path(buffer + 5);
        if (ss_number != -1)
        {
            send_to_client(accept_status_1, ss_number);
        }
    }
    else if (strncmp(buffer, "LIST", 4) == 0)
    {
        print_all_accessible_paths();
    }
    else if (strncmp(buffer, "WRITE", 5) == 0)
    {
        int ss_number = file_path(buffer + 6);
        if (ss_number != -1)
        {
            send_to_client(accept_status_1, ss_number);
        }
    }
    else if (strncmp(buffer, "DELETE", 6) == 0)
    {
        int ss_number = file_path(buffer + 7);
        if (ss_number != -1)
        {
            send_to_client(accept_status_1, ss_number);
        }
    }
    else if (strncmp(buffer, "CREATE", 6) == 0)
    {
        char temp_store[1000];
        strcpy(temp_store, buffer + 7);
        char *tokens[100];
        tokenize(temp_store, " ", tokens);
        for(int i=0;tokens[i]!=NULL;i++)
        {
            printf("Token is : %s\n",tokens[i]);
        }   
        int ss_number = file_path(tokens[0]);
        if (ss_number != -1)
        {
            send_to_storage_server(ss_number, tokens[0],tokens[1]);
        }
    }
    else if (strncmp(buffer, "ADDITIONAL_INFO", 15) == 0)
    {
        // exit(1);
    }
    else if (strncmp(buffer, "AUDIO_FILES/STREAM", 11) == 0)
    {
        // exit(1);
    }
    else if (strncmp(buffer, "COPY", 4) == 0)
    {
        char temp_store[1000];
        strcpy(temp_store, buffer + 5);
        char *tokens[100];
        tokenize(temp_store, " ", tokens);
        for(int i=0;tokens[i]!=NULL;i++)
        {
            printf("Token is : %s\n",tokens[i]);
        }
        int ss_number = file_path(tokens[0]);
        int ss_number_1 = file_path(tokens[1]);
        if (ss_number != -1 && ss_number_1 != -1)
        {
            send_to_storage_server(ss_number, tokens[0],tokens[2]);
            send_to_storage_server(ss_number_1, tokens[1],tokens[2]);
        }
    }
    else if (strncmp(buffer, "SERVER", 6) == 0)
    {
        add_storage_server(accept_status_1);
    }
    else
    {
        printf("Invalid request\n");
    }
    return NULL;
}

void *Handle_client_requests(void *arg)
{
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
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

void free_all()
{
    free(trie_root);
}

int main()
{
    initialze_storage_server();
    pthread_t client_thread;
    pthread_create(&client_thread, NULL, Handle_client_requests, NULL);
    pthread_join(client_thread, NULL);
    free_all();
    return 0;
}