    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <unistd.h>
    #include <arpa/inet.h>

    #define PORT 8080
    #define BUFFER_SIZE 1024

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

        if (strncmp(command, "stream_audio", 12) == 0) {
            play_audio_stream(client_socket);
        } else if (strncmp(command, "write", 5) == 0) {
        printf("Enter text to write to the file (type $STOP to end):\n");
        char input[BUFFER_SIZE];
        
        while (1) {
            // Get user input
            fgets(input, BUFFER_SIZE, stdin);

            // Check for $STOP to end the write session
            if (strcmp(input, "$STOP\n") == 0) {  // Include '\n' when checking for $STOP
                break;
            }

            // Send input to server (newline included)
            if (send(client_socket, input, strlen(input), 0) == -1) {
                perror("Send failed");
                break;  // Exit the loop if the send operation fails
            }
        }

        printf("File write operation completed.\n");
    } else {
            int bytes_received;
            while ((bytes_received = recv(client_socket, response, BUFFER_SIZE, 0)) > 0) {
                response[bytes_received] = '\0';
                printf("%s", response);
            }
        }

        close(client_socket);
        return 0;
    }

    // Function to play audio stream using mpv
    void play_audio_stream(int server_socket) {
        FILE *mpv_pipe;
        char buffer[BUFFER_SIZE];
        int bytes_received;

        // Open a pipe to mpv for audio playback
        mpv_pipe = popen("mpv --no-terminal --quiet -", "w");
        if (mpv_pipe == NULL) {
            perror("Failed to open mpv pipe");
            return;
        }

        printf("Streaming audio... Press Ctrl+C to stop.\n");

        // Receive and write audio data to mpv
        while ((bytes_received = recv(server_socket, buffer, BUFFER_SIZE, 0)) > 0) {
            fwrite(buffer, 1, bytes_received, mpv_pipe);
            fflush(mpv_pipe);
        }

        // Close the pipe when the stream ends
        pclose(mpv_pipe);
        printf("Audio stream ended.\n");
    }
