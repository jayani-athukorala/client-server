//This is the server program

#include "../include/client.h"

// Function to display errors
int check_error(int expression, const char *error_msg) {
    if (expression == FOUNDERROR) {
        perror(error_msg);
        exit(1);
    }
}

// Function to save received file on the client side
void write_file(int client_socket, FILE *fp) {
    ssize_t receive_status;
    char receive_data[MAX_BUFFER_SIZE] = {0};
    uint32_t filesize, write_size = 0;

    check_error(receive_status = recv(client_socket, &filesize, sizeof(uint32_t), 0), "RECEIVE ERROR");

    do {
        check_error(receive_status = recv(client_socket, receive_data, MAX_BUFFER_SIZE, 0), "RECEIVE ERROR");
        fwrite(receive_data, sizeof(char), receive_status, fp);
        memset(receive_data, 0, MAX_BUFFER_SIZE);
        write_size = write_size + receive_status;
    } while (filesize > write_size);

    return;
}

//Main function
int main(int argc, char *argv[]) {

    int client_socket, valread, my_id;
    char buffer[MAX_BUFFER_SIZE] = {0};
    ssize_t connect_status, send_status, receive_status;
    struct sockaddr_in remote_server_addr;
    FILE *fp;
    char receive_data[MAX_BUFFER_SIZE] = {0};
    uint32_t filesize, write_size = 0;

    // Create and set up the socket
    check_error(client_socket = socket(AF_INET, SOCK_STREAM, 0), "SOCKET ERROR");

    memset(&remote_server_addr, '\0', sizeof(remote_server_addr));
    remote_server_addr.sin_family = AF_INET;                 // Internet protocol AF_INET
    remote_server_addr.sin_port = htons(atoi(argv[4]));      // Address port
    remote_server_addr.sin_addr.s_addr = inet_addr(argv[2]); // Internet address
    int message_id = 0;

    // Establish a connection with the server
    check_error(connect_status = connect(client_socket, (struct sockaddr *)&remote_server_addr, sizeof(remote_server_addr)), "CONNECT ERROR");
    printf("Connected to server\n");

    // Handle communication with the server
    while (1) {
        // Send a message to the server
        printf("Enter message: ");
        char *val = fgets(buffer, MAX_BUFFER_SIZE, stdin);

        // Send user request to the server via socket
        check_error(send_status = send(client_socket, buffer, strlen(buffer), 0), "SEND ERROR");
        memset(buffer, 0, sizeof(buffer));

        // Receive assigned id and file name created
        if (check_error(receive_status = recv(client_socket, &my_id, sizeof(int), 0), "RECEIVE ERROR")) {

            // Create a folder for client connection if it does not exist
            snprintf(buffer, sizeof(buffer), "results/client%d", my_id);
            mkdir(buffer, 0777);
            bzero(buffer, MAX_BUFFER_SIZE);

            snprintf(buffer, sizeof(buffer), "results/client%d/message%d.txt", my_id, ++message_id);

            // Receive the file from the server
            fp = fopen(buffer, "wb");
            if (fp == NULL) {
                printf("WRITE ERROR");
                exit(1);
            }
            check_error(receive_status = recv(client_socket, &filesize, sizeof(uint32_t), 0), "RECEIVE ERROR");
            do {
                check_error(receive_status = recv(client_socket, receive_data, MAX_BUFFER_SIZE, 0), "RECEIVE ERROR");
                fwrite(receive_data, sizeof(char), receive_status, fp);
                memset(receive_data, 0, MAX_BUFFER_SIZE);
                write_size = write_size + receive_status;
            } while (filesize > write_size);

            fclose(fp);
            printf("File received and saved as message%d.txt in client%d folder\n", message_id, my_id);
        }
    }

    close(client_socket);
    return 0;
}
