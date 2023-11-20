// This is the server program which handles mulptiple requests of multiple clients using different strategies

#include "../include/server.h"

char* STRATERGY = "fork";

//This function display errors
int check_error(int expression, const char *error_msg){
	if(expression == FOUNDERROR){
		perror(error_msg);
		exit(EXIT_FAILURE);			
	}
}

//This function handle single client
void handle_client(int client_socket, int client_id) {

    ssize_t receive_status, send_status;
    char buffer[MAX_BUFFER_SIZE] = {0};
    FILE *fp;
    struct stat st;                     //stat structure
    uint32_t filesize; 

    // Handle communication with the client
    while(1){
        
        // Receive message from the client
        if(check_error(read(client_socket, buffer, sizeof(buffer)), "Message Error")){
            // Receive message from the client
            printf("Received message from client%d: %s\n", client_id, buffer);

            // Write the received message to a text file
            FILE *file = fopen("temp_message.txt", "w");
            if (file == NULL) {
                perror("Error opening file");
                exit(EXIT_FAILURE);
            }

            fprintf(file, "%s", buffer);
            fclose(file);

            printf("Message written to temp_message.txt\n");

            check_error(send_status = send(client_socket, &client_id, sizeof(int), 0),"SEND ERROR\n");

            fp = fopen("temp_message.txt", "rb");
	
			if(fp == NULL){
				printf("READ ERROR");
				exit(1);
			} 
		
			stat("temp_message.txt", &st);
			//getting file size in network order byte
    		filesize = st.st_size;
			//sending file size to client
    		check_error(send(client_socket, &filesize, sizeof(uint32_t), 0),"SEND ERROR");
			//send result file to client
			//read file to buffer
            char send_data[MAX_BUFFER_SIZE] = {0};
            while((send_status = fread(send_data, sizeof(char), MAX_BUFFER_SIZE, fp)) > 0) {
                //send file via socket
                check_error(send(client_socket, send_data, send_status, 0),"SEND ERROR");
                memset(send_data, 0, MAX_BUFFER_SIZE);
            }

			fclose(fp);

            printf("Created file sent to the client %d.\n", client_id);

        }        

    }

    // Close the client socket when done
    close(client_socket);
    return;
}

// This function create clients using fork()
void fork_clients(int server_socket,int port){
    int client_id = 0;
    
    while (1) {
        
        int client_socket;
        pid_t pid;

        // Accept a client connection
        if(check_error(client_socket = accept(server_socket, NULL, NULL), "Error accepting connection")){
            
            printf("Connected with Client %d \n", ++client_id);
        }

        // Fork a new process to handle the client
        if(check_error(pid = fork(), "Error forking")){
            close(client_socket);
        } else if (pid == 0) {
            // Child process (handles the client)
            close(server_socket);  // Close the server socket in the child process
            handle_client(client_socket, client_id);
            exit(EXIT_SUCCESS);
        } else {
            // Parent process (continues listening for new connections)
            close(client_socket);  // Close the client socket in the parent process
        }
    }
    // Close the server socket
    close(server_socket);

}

// This function connects multiple clients using select()
int select_clients(int server_socket, int port){
    
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(server_socket, &read_fds);
    int client_id =0;

    int max_fd = server_socket;
    int client_array[max_fd];

    while (1) {
        fd_set tmp_fds = read_fds;

        // Use select() to monitor file descriptors for readability
        check_error(select(max_fd + 1, &tmp_fds, NULL, NULL, NULL), "Error in select"); 

        // Check if the server socket is ready for a new connection
        if (FD_ISSET(server_socket, &tmp_fds)) {
            int client_socket;

            if (check_error(client_socket = accept(server_socket, NULL, NULL),"Error accepting connection")) {
                
                // Add the new client socket to the set
                FD_SET(client_socket, &read_fds);

                // Update max_fd if necessary
                if (client_socket > max_fd) {
                    max_fd = client_socket;
                }
                client_array[client_socket] = ++client_id;
                printf("Connected with %d, socket: %d\n", client_id, client_socket);
            }
        }

        
        // Check each client socket for data
        for (int i = 0; i <= max_fd; ++i) {
            
            if (FD_ISSET(i, &tmp_fds) && i != server_socket) {
                // Handle the client
                printf("hiii\n");
                printf("%d", client_array[i]);
                handle_client(i, client_array[i]);

                // Remove the client socket from the set
                FD_CLR(i, &read_fds);
                close(i);

                printf("Client %d disconnected, socket: %d\n",client_id--, i);
            }
        }
    }
    // Close the server socket 
    close(server_socket);

}

// This function connect multiple clients using poll()
int poll_clients(int server_socket, int port){

    struct pollfd fds[MAX_CLIENTS + 1];  // +1 for the server socket
    memset(fds, 0, sizeof(fds));

    fds[0].fd = server_socket;
    fds[0].events = POLLIN;

    int num_clients = 0, client_id = 0;

    while (1) {
        // Use poll() to monitor file descriptors for events
        check_error(poll(fds, num_clients + 1, -1),"Error in poll");

        // Check if the server socket is ready for a new connection
        if (fds[0].revents & POLLIN) {
            int client_socket;
            
            if (check_error(client_socket = accept(server_socket, NULL, NULL),"Error accepting connection")) {

                // Add the new client socket to the array
                fds[num_clients + 1].fd = client_socket;
                fds[num_clients + 1].events = POLLIN;
                num_clients++;

                printf("Connected with client %d, socket: %d\n", ++client_id, client_socket);
            }
        }

        // Check each client socket for data
        for (int i = 1; i <= num_clients; ++i) {
            if (fds[i].revents & POLLIN) {
                // Handle the client
                handle_client(fds[i].fd, client_id);

                // Remove the client socket from the array
                close(fds[i].fd);
                fds[i] = fds[num_clients];
                num_clients--;

                printf("Connected with client %d, socket: %d\n", ++client_id, fds[i].fd);
            }
        }
    }
    // Close the server socket 
    close(server_socket);

}

// This function connect multiple clients using epoll()
int epoll_clients(int server_socket, int port){
    // Create an epoll instance
    int epoll_fd;
    
    check_error(epoll_fd = epoll_create1(0), "Error creating epoll");

    struct epoll_event ev, events[MAX_EVENTS];
    ev.events = EPOLLIN;
    ev.data.fd = server_socket;
    int client_id = 0;

    // Add the server socket to the epoll set
    check_error(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_socket, &ev), "Error adding server socket to epoll");

    while (1) {

        int num_events;

        // Wait for events using epoll_wait()   
        check_error(num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1),"Error in epoll_wait");

        for (int i = 0; i < num_events; ++i) {
            if (events[i].data.fd == server_socket) {
                // New connection
                int client_socket;
                
                if (check_error(client_socket = accept(server_socket, NULL, NULL), "Error accepting connection")) {
             
                    // Add the new client socket to the epoll set
                    ev.events = EPOLLIN;
                    ev.data.fd = client_socket;
                    if (check_error(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket, &ev),"Error adding client socket to epoll")) {
                        close(client_socket);
                    }

                    printf("Connected with client %d, socket: %d\n", ++client_id, client_socket);
                }
            } else {
                // Data available on a client socket
                if (events[i].events & EPOLLIN) {
                    handle_client(events[i].data.fd, client_id);

                    // Remove the client socket from the epoll set
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                    close(events[i].data.fd);

                    printf("Client %d disconnected, socket: %d\n", client_id--, events[i].data.fd);
                }
            }
        }
    }
    // Close the server socket and epoll instance
    close(server_socket);
    close(epoll_fd);

}

// This is the main function
int main(int argc, char *argv[]) {

    // Create and set up the socket
    int server_socket;
    
    check_error(server_socket = socket(AF_INET, SOCK_STREAM, 0), "Error creating socket");

    // Set up the server address struct
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the socket
    check_error(bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)),"Error binding socket");

    // Listen for incoming connections
    check_error(listen(server_socket, MAX_CLIENTS),"Error listening");

    printf("Server listening on port %d...\n", PORT);

    // Handle different strategies
    
    if (strcmp(STRATERGY, "select") == 0) {
        // Handle clients using select()
        select_clients(server_socket, PORT);
    } else if (strcmp(STRATERGY, "poll") == 0) {
        // Handle clients using poll()
        poll_clients(server_socket, PORT);
    } else if (strcmp(STRATERGY, "epoll") == 0) {
        // Handle clients using epoll()
        epoll_clients(server_socket, PORT);
    } else {
        // Handle the default case
        // Handle clients using fork()
        fork_clients(server_socket, PORT);
    }

    return 0;
}
