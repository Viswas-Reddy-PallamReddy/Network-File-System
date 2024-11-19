#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include "erro_code.h"
#include <arpa/inet.h>
#define CHUNK_SIZE 256 // Define the size of each data chunk
#define MAX_ACCESSIBLE_PATHS 100
#define MAX_FILE_SIZE 4096
#define MAX_CHUNK_WRITE 40
#define THRESHOLD 20

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
void handle_ss(char *input,char *buff,int nm_socket);
void play_mp3(const char *filename);
void *write_thread(void *arg);

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
            handle_ss(input,buffer,nm_socket);
            close(nm_socket);
            return;
        }
        if(strncmp(buffer,"ERROR",5)==0)
        {
            int error_code;
            sscanf(buffer, "ERROR %d", &error_code);
            if(error_code!=0)
                printf("Naming Server: Error %d : %s\n", error_code,error_message(error_code));
            else
                printf("Naming Server: %s\n", error_message(error_code)); 
        }
        else
            printf("Naming Server: \n%s\n", buffer);
    }
    // PENDING CODE FOR CREATE ,DELETE, LIST BY CHECkING NM CODE
    
}
void handle_ss(char *input,char *buff,int nm_socket)
{
    char ss_ip[INET_ADDRSTRLEN];
    int ss_port;
    if(strncmp(buff,"IP",2)!=0)
    {
        int error_code;
        sscanf(buff, "ERROR %d\n", &error_code);
        printf("Error %d : %s\n", error_code,error_message(error_code));
        // printf("Error : %s\n", buff);
        return;
    }
    printf("%s\n",buff);
    sscanf(buff, "IP %s PORT %d", ss_ip, &ss_port);
    // Connect to storage server
    // ss_port=5837;
    // sprintf(ss_ip, "%s", "10.42.0.89");
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
        int bytes_received;
        memset(&pkt,0,sizeof(pkt));
        bytes_received = recv(ss_socket, &pkt, sizeof(pkt), 0);
        if (bytes_received > 0)
        {
            printf("%s\n", pkt.data);
        }
        int total_chunks_q=pkt.total_chunks;
        printf("Total chunks are %d\n",total_chunks_q);
        for(int i=1;i<total_chunks_q;i++)
        {
            memset(&pkt,0,sizeof(pkt));
            bytes_received = recv(ss_socket, &pkt, sizeof(pkt), 0);
            if (bytes_received > 0)
            {
                // printf("\n\nChunk %d\n\n",pkt.seq_num);
                printf("%s\n", pkt.data);
            }
        }

        int error_c;
        recv(ss_socket, &error_c, sizeof(error_c), 0);
        if(error_c==0)
            printf("%s\n", error_message(error_c));
        else
            printf("ERROR %d : %s\n",error_c,error_message(error_c));
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
        char buffer[BUFFER_SIZE];
        recv(ss_socket, buffer, BUFFER_SIZE, 0);
        if(strncmp(buffer,"ERROR",5)==0)
        {
            int error_code;
            sscanf(buffer,"ERROR %d", &error_code);
            printf("ERROR %d : %s\n",error_code,error_message(error_code));
            goto last;
        }
        else
            printf("%s\n",buffer);
        packet write_pkt[MAX_CHUNK_WRITE];
        // char data[CHUNK_SIZE];
        int seq_num=0;
        for(int i=0;i<MAX_CHUNK_WRITE;i++)
        {
            // memset(data,0,CHUNK_SIZE);
            memset(&write_pkt[i],0,sizeof(write_pkt[i]));
            fgets(write_pkt[i].data, CHUNK_SIZE, stdin);
            if(strncmp(write_pkt[i].data,"$STOP",5)==0)
            {
                write_pkt[i].seq_num=-1;
                // send(ss_socket, &pkt, sizeof(pkt), 0);
                break;
            }
            // seq_num++;
            write_pkt[i].seq_num=seq_num;
            seq_num++;
            // send(ss_socket, &pkt, sizeof(pkt), 0);
        }
        printf("Sending %d packets\n",seq_num);
        for(int i=0;i<MAX_CHUNK_WRITE;i++)
        {
            // printf("Sending packet: %s\n",write_pkt[i].data);
            send(ss_socket, &write_pkt[i], sizeof(write_pkt[i]), 0);
            if(write_pkt[i].seq_num==-1)
                break;
        }
        // char buffer[BUFFER_SIZE];
        char buffer2[BUFFER_SIZE];
        int bytes_received = recv(ss_socket, buffer2, BUFFER_SIZE, 0);
        if (bytes_received > 0)
            buffer2[bytes_received] = '\0';
        if(strncmp(buffer2,"request has been accepted",25)==0)
        {
            printf("request has been accepted\n");
            pthread_t thread_id;
            pthread_create(&thread_id, NULL, write_thread, &ss_socket);
        }
        else
        {
            int error_code;
            sscanf(buffer2,"ERROR %d", &error_code);
            if(error_code==0)
                printf("%s\n", error_message(error_code));
            else    
                printf("ERROR %d : %s\n",error_code,error_message(error_code));
        }
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
    FILE *ffplaypipe=popen("ffplay -nodisp -autoexit -", "w");
    if(ffplaypipe==NULL)
    {
        printf("ffplay failed\n");
        return;
    }

    char buffer[256];
    ssize_t bytes_received;
    int firstTime=1;
    while(1)
    {
        bytes_received = recv(server_socket, buffer, 256, 0);
        if (bytes_received <= 0)
        {
            if(bytes_received==0)
                printf("End of Stream\n");
            else
            {
                if(firstTime==1)
                    printf("File not Found\n"); 
                else
                    printf("Error\n");    
            }
            break;        
        }
        if(fwrite(buffer, 1, bytes_received, ffplaypipe) != (size_t)bytes_received)
        {
            printf("Error writing to pipe\n");
            break;
        }
        fflush(ffplaypipe);
        firstTime=0;
    }
    pclose(ffplaypipe);
}

void* write_thread(void *arg) {
    // printf("Thread started\n");
    int ss_socket = *(int *)arg;
    int code;
    recv(ss_socket, &code, sizeof(code), 0);
    if(code==0)
        printf("%s\n", error_message(code));
    else
        printf("ERROR %d : %s\n",code,error_message(code));   
    close(ss_socket);     
    pthread_exit(NULL);
}
