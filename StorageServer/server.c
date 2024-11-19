#include "headers.h"
#include "SS_client.h"
#include "SS_NM.h"

StorageServer this;
pthread_t nm_thread;
pthread_t client_thread;
int final_nm_socket;

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "123 Usage: %s <NM_IP> %s <NM_PORT>\n", argv[0],argv[1]);
        exit(EXIT_FAILURE);
    }

    int nm_port = atoi(argv[1]);
    const char* ip_address = argv[2];
    int nm_socket;
    nm_socket = connect_to_nm(nm_port,ip_address);
    if (nm_socket < 0) {
        fprintf(stderr, "134 : Failed to connect to Naming Server\n");
        exit(EXIT_FAILURE);
    }
    const char* my_ip = "10.42.0.89";

    initialise_to_nm(nm_socket,my_ip);


    if (pthread_create(&nm_thread, NULL, handle_nm_thread, &nm_socket) != 0) {
        perror("Failed to create NM thread");
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&client_thread, NULL, handle_client_thread, NULL) != 0) {
        perror("Failed to create Client thread");
        exit(EXIT_FAILURE);
    }

    pthread_join(nm_thread, NULL);
    pthread_join(client_thread, NULL);

    close(nm_socket);
    return 0;
}

