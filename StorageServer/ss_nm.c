#include "headers.h"
#include "SS_NM.h"

#define NM_PORT 5232



int connect_to_nm(int nm_port,const char* ip_address) {
    int nm_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in nm_addr;
    nm_addr.sin_family = AF_INET;
    nm_addr.sin_port = htons(nm_port);
    
    if (inet_pton(AF_INET, ip_address, &nm_addr.sin_addr) <= 0) {
        perror("133 : Invalid IP address format for Naming Server");
        close(nm_socket);
        return -1;
    }
    if (connect(nm_socket, (struct sockaddr*)&nm_addr, sizeof(nm_addr)) < 0) {
        perror("134 : Failed to connect to Naming Server");
        return -1;
    }
    return nm_socket;
}

void initialise_to_nm(int nm_socket,const char* ip_address){
    send(nm_socket, "SERVER", 6, 0);
    usleep(5000);
    strcpy(this.ip,ip_address);
    this.nm_port = NM_PORT;
    this.client_port = CLIENT_PORT;
    this.num_accessible_paths = 0;
    for(int i = 0; i < this.num_accessible_paths; i++){
        strcpy(this.accessible_paths[i],"");
    }
    
    char base[1024];
    getcwd(base, sizeof(base));
    printf("base dir: %s\n",base);
    strcat(base,"/Paths");
    traverse_directory(base, base);
    printf("num_accessible_paths: %d\n",this.num_accessible_paths);
    char temp[MAX_PATH_LEN*this.num_accessible_paths];
    char details[1024];
    sprintf(details, "%s %d %d %d",this.ip,this.nm_port,this.client_port,this.num_accessible_paths);
    send(nm_socket, details,sizeof(details),0);
    usleep(5000);
    for(int i = 0; i < this.num_accessible_paths; i++){
        printf("%s\n",this.accessible_paths[i]);
        strcat(temp,this.accessible_paths[i]);
        if(i!=this.num_accessible_paths-1)
            strcat(temp,"\n");
        
    }
    send(nm_socket, temp,sizeof(temp),0);
    // close(nm_socket);
}

void *handle_nm_thread(void *arg) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int nm_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (nm_socket < 0)
    {
        perror("137: Error in creating nm socket");
        exit(1);
    }
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(NM_PORT);
    client_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(nm_socket, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0)
    {
        perror("138 :Error in binding nm socket");
        exit(1);
    }
    int listen_status = listen(nm_socket,100);
    if (listen_status < 0)
    {
        perror("139 : Error in listening to nm socket");
        exit(1);
    }
    while (1)
    {
        printf("Waiting for nm connection\n");
        final_nm_socket = accept(nm_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (final_nm_socket < 0)
        {
            perror("Error in accepting connection from Naming Server");
            exit(1);
        }
        pthread_t thread;
        pthread_create(&thread, NULL, process_requests, NULL);
        // pthread_detach(thread);
    }
}

void *process_requests(void *arg) {

    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    while(1){
        int bytes_received = recv(final_nm_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received < 0) {
            perror("Failed to receive command from NM");
            close(final_nm_socket);
            break;
        }
        printf("Received command:%s\n",buffer);
        buffer[bytes_received] = '\0';
        if(strcmp(buffer,"EXIT")==0){
            printf("Received 'exit' command from NM. Shutting down NM thread...\n");
            break;
        }
        else{
            handle_nm(buffer);
        }
        // send(nm_socket, "Received", strlen("Received"), 0);
    }
    printf("NM thread exiting...\n");
    pthread_exit(NULL);
}

void handle_nm(char *nm_command) {
    // printf("Received command from nm: %s\n", nm_command);
    char* command = strtok(nm_command, " ");
    printf("command: %s\n",command);
    if(strcmp(command, "CREATE")==0){
        char* type = strtok(NULL, " ");
        printf("type: %s\n",type);
        char* temp = strtok(NULL, " ");
        printf("temp: %s\n",temp);
        char  path[MAX_PATH_LEN];
        snprintf(path, strlen(temp)+9, "./Paths/%s/", temp+2);
        printf("path: %s\n",path);
        char* filename = strtok(NULL, " ");
        strcat(path, filename);
        printf("filename: %s\n",filename);
        if(strcmp(type, "file")==0){
            create_file(path);
        }
        else if(strcmp(type, "folder")==0){
            create_folder(path);
        }
    }
    else if(strcmp(command, "DELETE")==0){
        char* temp = strtok(NULL, " ");
         char filename[BUFFER_SIZE];
        snprintf(filename, BUFFER_SIZE, "Paths/%s", temp+2);
        printf("file to delete:%s\n",filename);
        DIR* dir = opendir(filename);
        if (dir == NULL) {
            delete_file(filename);
        } else {
            delete_folder(filename);
        }
        closedir(dir);
    }
    else if(strcmp(command, "READ")==0){
        char* temp = strtok(NULL, " ");
        char filename[BUFFER_SIZE];
        snprintf(filename, BUFFER_SIZE, "Paths/%s", temp+2);
        read_file(final_nm_socket,filename);
    }
    else if(strcmp(command, "WRITE")==0){
        char* temp = strtok(NULL, " ");
        char filename[BUFFER_SIZE];
        snprintf(filename, BUFFER_SIZE, "Paths/%s", temp+2);
        write_to_file(final_nm_socket,filename,1);
    }
    else{
        send(final_nm_socket, "ERROR 1", 10, 0);
    }

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
                    snprintf(relative_path, sizeof(relative_path), "%s", ent->d_name);
                } else {
                    snprintf(relative_path, sizeof(relative_path), "./%s", full_path + strlen(base) + 1);
                }

                // Recursive call for subdirectories
                if (S_ISDIR(st.st_mode)) {
                    // strcat(relative_path, " folder");
                    traverse_directory(full_path, base);
                }
                // else{
                //     strcat(relative_path, " file");
                // }
                strcpy(this.accessible_paths[this.num_accessible_paths], relative_path);
                this.num_accessible_paths++;
            }
        }
        closedir(dir);
    } else {
        perror("Error reading directory");
    }
}

void create_file(const char *filepath) {
    FILE *temp_file = fopen(filepath, "r");
    if (temp_file != NULL) {
        printf("File already exists\n");
        // send(nm_socket, "File already exists\n", 20, 0);
        send(final_nm_socket, "ERROR 5", 10, 0);
        fclose(temp_file);
        return; 
    }
    // fclose(temp_file);
    printf("Creating file: %s\n", filepath);
    FILE *file = fopen(filepath, "w");
    if (!file) {
        printf("Failed to create file\n");
        // send(nm_socket, "Failed to create file\n", 23, 0);
        send(final_nm_socket, "ERROR 7", 10, 0);
        return;
    }
    printf("File created successfully\n");
    // send(nm_socket, "File created successfully\n", 26, 0);
    send(final_nm_socket, "ERROR 0", 10, 0);
    fclose(file);
}

void delete_file( const char *filename) {
    printf("Deleting file: %s\n", filename);
    char buffer[BUFFER_SIZE];
    if (remove(filename) == 0) {
        snprintf(buffer, BUFFER_SIZE, "Deleted file: %s successfully\n", filename);
        printf("Deleted file: %s successfully\n", filename);
        // send(nm_socket, buffer, BUFFER_SIZE, 0);
        send(final_nm_socket, "ERROR 0", 10, 0);
    } else {
        // send(nm_socket, "Failed to delete file\n", 22, 0);
        printf("Failed to delete file\n");
        send(final_nm_socket, "ERROR 6", 10, 0);
    }
}


void create_folder(const char* filepath){
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    int status = mkdir(filepath, 0777);
    if (status == -1) {
        snprintf(buffer, sizeof(buffer), "Failed to create folder %s\n", filepath);
        // send(nm_socket, buffer, strlen(buffer), 0);
        send(final_nm_socket, "ERROR 7", 10, 0);
        printf("Error creating folder %s\n", filepath);
    } else {
        snprintf(buffer, sizeof(buffer), "Folder %s created successfully\n", filepath);
        // send(nm_socket, buffer, strlen(buffer), 0);
        send(final_nm_socket, "ERROR 0", 10, 0);
        printf("Folder %s created successfully\n", filepath);
    }
}

void delete_folder( const char* filepath){
    printf("Deleting folder: %s\n", filepath);
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
            delete_folder(full_path);
        } else {
            delete_file(full_path);
        }
    }

    closedir(dir);
    int status = rmdir(filepath);
    char buffer[BUFFER_SIZE];
    if (status == -1) {
        snprintf(buffer, BUFFER_SIZE, "Failed to delete folder %s\n", filepath);
        // send(nm_socket, buffer, strlen(buffer), 0);
        send(final_nm_socket, "ERROR 6", 10, 0);
        printf("Error deleting folder %s\n", filepath);
    } else {
        // snprintf(buffer, BUFFER_SIZE, "Folder %s deleted successfully\n", filepath);
        // send(nm_socket, buffer, strlen(buffer), 0);
        send(final_nm_socket, "ERROR 0", strlen("ERROR 0"), 0);
        // printf("%s",buffer);
        printf("Folder %s deleted successfully\n", filepath);
    }
}
