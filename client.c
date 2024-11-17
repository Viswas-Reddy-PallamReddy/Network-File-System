#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#define CHUNK_SIZE 256 // Define the size of each data chunk
#define MAX_ACCESSIBLE_PATHS 100
#define MAX_FILE_SIZE 4096

// Define the packet structure
typedef struct packet
{
    int seq_num;           // Sequence number of the packet
    int total_chunks;      // Total number of chunks
    char data[CHUNK_SIZE]; // Data chunk
} packet;

char *NM_IP;
int NM_PORT;
#define BUFFER_SIZE 1024

void play_audio_stream(int server_socket);
void handle_nm(char *input,int nm_socket);
void handle_ss(char *input,char *buff);

int main(int argc,char* argv[])
{
    if(argc<3)
    {
        fprintf(stderr, "Usage: %s <port> <ip>\n", argv[0]);
        return 1;
    }
    NM_IP = argv[2];
    NM_PORT = atoi(argv[1]);
    printf("%s %d\n",NM_IP,NM_PORT);
   
    while(1)
    {
        int nm_socket;
        struct sockaddr_in nm_addr;
        char buffer[BUFFER_SIZE];

        // Create socket
        nm_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (nm_socket == -1)
        {
            perror("Socket creation failed");
            exit(EXIT_FAILURE);
        }

        // Set up naming server address
        nm_addr.sin_family = AF_INET;
        nm_addr.sin_port = htons(NM_PORT);
        nm_addr.sin_addr.s_addr = inet_addr(NM_IP);

        // Connect to naming server
        if (connect(nm_socket, (struct sockaddr *)&nm_addr, sizeof(nm_addr)) < 0)
        {
            perror("Connection to naming server failed");
            close(nm_socket);
            exit(EXIT_FAILURE);
        }
        char input[BUFFER_SIZE];
        printf("Enter command (read, write, delete, create, list, stream) and path: ");
        if (fgets(input, BUFFER_SIZE, stdin) != NULL)
        {
            input[strcspn(input, "\n")] = '\0';
            if(strncmp(input,"EXIT",4)==0)
                break;
            handle_nm(input,nm_socket);
        }
        close(nm_socket);
    }
    return 0;
}

void handle_nm(char *input,int nm_socket)
{
    char buffer[BUFFER_SIZE];
    // Send command to naming server
    snprintf(buffer, BUFFER_SIZE, "%s", input);
    send(nm_socket, buffer, strlen(buffer), 0);
    char temp_store[MAX_ACCESSIBLE_PATHS][MAX_FILE_SIZE];
    // Receive response from naming server
    int bytes_received;
    bytes_received = recv(nm_socket, buffer, BUFFER_SIZE, 0);

    if (bytes_received > 0)
    {
        buffer[bytes_received] = '\0';
        if(strncmp(input,"READ",4)==0 || strncmp(input,"WRITE",5)==0 || strncmp(input,"STREAM",6)==0 || strncmp(input,"GET_INFO",8)==0)
        {
            handle_ss(input,buffer);
            close(nm_socket);
            return;
        }
        printf("Naming Server: %s\n", buffer);
    }
    // PENDING CODE FOR CREATE ,DELETE, LIST BY CHECkING NM CODE
    
}
void handle_ss(char *input,char *buff)
{
    char ss_ip[INET_ADDRSTRLEN];
    int ss_port;
    printf("%s\n",buff);
    sscanf(buff, "%s %d", ss_ip, &ss_port);
    // Connect to storage server
    int ss_socket;
    struct sockaddr_in ss_addr;

    ss_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (ss_socket == -1)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    ss_addr.sin_family = AF_INET;
    ss_addr.sin_port = htons(ss_port);
    ss_addr.sin_addr.s_addr = inet_addr(ss_ip);

    if (connect(ss_socket, (struct sockaddr *)&ss_addr, sizeof(ss_addr)) < 0)
    {
        perror("Connection to storage server failed");
        close(ss_socket);
        exit(EXIT_FAILURE);
    }
    send(ss_socket, input, strlen(input), 0);
    if(strncmp(input,"READ",4)==0)
    {
        packet pkt;
        while(1)
        {   
            memset(&pkt,0,sizeof(pkt));
            int bytes_received;
            if((bytes_received = recv(ss_socket, &pkt, sizeof(pkt), 0)) > 0)
            {
                if(strncmp(pkt.data,"$STOP",5)==0 || pkt.seq_num==-1)
                {
                    break;
                }
                printf("%s\n",pkt.data);
            }
            else
            {
                break;
            }
        }
    }
    else if(strncmp(input,"GET_INFO",8)==0)
    {
        char buffer[BUFFER_SIZE];
        int bytes_received = recv(ss_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received > 0)
        {
            buffer[bytes_received] = '\0';
            printf("INFO:\n%s\n", buffer);
        }
    }
    else if(strncmp(input,"WRITE",5)==0)
    {
        packet pkt;
        int bytes_received = recv(ss_socket, &pkt, sizeof(pkt), 0);
        if (bytes_received > 0)
        {
            printf("%s\n", pkt.data);
            if(pkt.seq_num==-1)
            {
                goto last;
            }
        }
        char data[CHUNK_SIZE];
        int seq_num=0;
        while(1)
        {
            memset(data,0,CHUNK_SIZE);
            memset(&pkt,0,sizeof(pkt));
            fgets(data, CHUNK_SIZE, stdin);
            strcpy(pkt.data,data);
            if(strncmp(data,"$STOP",5)==0)
            {
                pkt.seq_num=-1;
                send(ss_socket, &pkt, sizeof(pkt), 0);
                break;
            }
            seq_num++;
            pkt.seq_num=seq_num;
            send(ss_socket, &pkt, sizeof(pkt), 0);
        }
        char buffer[BUFFER_SIZE];
        memset(buffer,0,BUFFER_SIZE);
        recv(ss_socket, buffer, BUFFER_SIZE, 0);
        printf("%s\n", buffer);
    }
    else if(strncmp(input,"STREAM",6)==0)
    {
        play_audio_stream(ss_socket);
    }
    last:
    close(ss_socket);
}
void play_audio_stream(int server_socket)
{
    FILE *mpv_pipe;
    char buffer[BUFFER_SIZE];
    int bytes_received;

    // Open a pipe to mpv for audio playback
    mpv_pipe = popen("mpv --no-terminal --quiet -", "w");
    if (mpv_pipe == NULL)
    {
        perror("Failed to open mpv pipe");
        return;
    }

    printf("Streaming audio... Press Ctrl+C to stop.\n");

    // Receive and write audio data to mpv
    while ((bytes_received = recv(server_socket, buffer, BUFFER_SIZE, 0)) > 0)
    {
        fwrite(buffer, 1, bytes_received, mpv_pipe);
        fflush(mpv_pipe);
    }

    // Close the pipe when the stream ends
    pclose(mpv_pipe);
    printf("Audio stream ended.\n");
}
