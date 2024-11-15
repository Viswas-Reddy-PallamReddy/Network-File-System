#include "headers.h"
#include "SS_client.h"


#define PORT 8080
#define BUFFER_SIZE 4096
#define CHUNK_SIZE 1024  // Define the size of each data chunk

// Define the packet structure
typedef struct packet {
    int seq_num;            // Sequence number of the packet
    int total_chunks;       // Total number of chunks
    char data[CHUNK_SIZE];  // Data chunk
} packet;

void play_audio_stream(int server_socket);

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    char command[BUFFER_SIZE], response[BUFFER_SIZE];

    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect to server
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    printf("Connected to the server. Enter command (read, get_info, write, stream_audio): ");
    fgets(command, BUFFER_SIZE, stdin);
    command[strcspn(command, "\n")] = 0;  // Remove newline

    // Send command to server
    send(client_socket, command, strlen(command), 0);

    if (strncmp(command, "stream", 6) == 0) {
        play_audio_stream(client_socket);
    } else if (strncmp(command, "write", 5) == 0) {
        printf("Enter text to write to the file (type $STOP to end):\n");
        char input[BUFFER_SIZE];
        packet pkt;
        int seq_num = 0;
        while (1) {
            // Get user input
            fgets(input, BUFFER_SIZE, stdin);
            // input[strcspn(input, "\n")] = 0;  // Remove newline
            // Check for $STOP to end the write session
            if (strcmp(input, "$STOP\n") == 0) {
                pkt.seq_num = -1;
                pkt.total_chunks = 0;
                strncpy(pkt.data, "END", CHUNK_SIZE);
                send(client_socket, &pkt, sizeof(pkt), 0);
                break;
            }
            pkt.seq_num = seq_num;
            pkt.total_chunks = 1;
            strncpy(pkt.data, input, CHUNK_SIZE);
            printf("packet data: %s\n",pkt.data);
            // Send packet data
            send(client_socket, &pkt, sizeof(pkt), 0);
            seq_num++;
            // Send input to server (newline included)
            if (send(client_socket, input, strlen(input), 0) == -1) {
                perror("Send failed");
                break;
            }
        }

        printf("File write operation completed.\n");
    }
    else if ((strncmp(command, "read", 4) == 0) || (strncmp(command, "get_info", 8) == 0)) {
        int bytes_received;
        packet pkt;
        while ((bytes_received = recv(client_socket, &pkt , sizeof(pkt), 0)) > 0) {
            // if(sizeof(pkt) == bytes_received) {
                // response[bytes_received] = '\0';
                printf("%s", pkt.data);
            // }
        }
    }
    else if(strncmp(command, "list", 4) == 0){
        StorageServer here;
        int bytes_received = recv(client_socket, &here, sizeof(StorageServer), 0);
        
        if (bytes_received <= 0) {
            perror("Failed to receive StorageServer data");
            return 0;
        }
        printf("Server IP: %s\n", here.ip);
        printf("Server port: %d\n", here.nm_port);
        printf("Client port: %d\n", here.client_port);  
        printf("num_accessible_paths: %d\n", here.num_accessible_paths);
        for(int i = 0; i < 20; i++){
            printf("%s\n", here.accessible_paths[i]);
        }
    }
    else if(strncmp(command, "create", 6) == 0){
        int bytes_received = recv(client_socket, response, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            perror("Failed to receive StorageServer data");
            return 0;
        }
        printf("%s", response);
        scanf("%s",response);
        send(client_socket, response, strlen(response), 0);
        printf("type: %s\n",response);
        char buff[BUFFER_SIZE];
        memset(buff, 0, BUFFER_SIZE);
        recv(client_socket,buff, BUFFER_SIZE, 0);
        printf("%s", buff);
    }
    else {
        int bytes_received;
        while ((bytes_received = recv(client_socket, response, BUFFER_SIZE, 0)) > 0) {
            response[bytes_received] = '\0';
            printf("%s", response);
        }
    }

    close(client_socket);
    return 0;
}

// Function to receive and play audio stream
void play_audio_stream(int server_socket) {
    packet pkt;
    const char *filename = "received_audio.mp3";
    FILE *audio_file = fopen(filename, "wb");

    if (!audio_file) {
        perror("File creation failed");
        return;
    }

    printf("Receiving audio stream...\n");

    int bytes_received;
    // long file_size = 0;
    while ((bytes_received = recv(server_socket, &pkt, sizeof(pkt), 0)) > 0) {
        if (bytes_received == sizeof(pkt)) {
            fwrite(pkt.data, 1, CHUNK_SIZE, audio_file);  // Exclude header data
        }
    }

    fclose(audio_file);
    printf("Audio stream received. Playing audio...\n");

    // Play the audio using mpv
    char command[256];
    snprintf(command, sizeof(command), "mpv %s", filename);
    int result = system(command);

    if (result != 0) {
        perror("Error playing audio file");
    }

    // Delete the audio file after playing
    // if (remove(filename) == 0) {
    //     printf("Audio file deleted successfully.\n");
    // } else {
    //     perror("Error deleting audio file");
    // }
}
