#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        memset(response, 0, sizeof(response));

        if (recv(client_socket, buffer, sizeof(buffer), 0) < 0) {
            perror("Error receiving data from client");
            break;
        }

        if (strncmp(buffer, "GET_TIME", 8) == 0) {
            char* format = buffer + 9;  // Skip "GET_TIME " in the received command
            struct tm* current_time;
            time_t now;
            time(&now);
            current_time = localtime(&now);

            if (strcmp(format, "dd/mm/yyyy") == 0) {
                strftime(response, sizeof(response), "%d/%m/%Y", current_time);
            }
            else if (strcmp(format, "dd/mm/yy") == 0) {
                strftime(response, sizeof(response), "%d/%m/%y", current_time);
            }
            else if (strcmp(format, "mm/dd/yyyy") == 0) {
                strftime(response, sizeof(response), "%m/%d/%Y", current_time);
            }
            else if (strcmp(format, "mm/dd/yy") == 0) {
                strftime(response, sizeof(response), "%m/%d/%y", current_time);
            }
            else {
                strcpy(response, "Invalid date format. Please try again.");
            }
        }
        else {
            strcpy(response, "Invalid command. Please use GET_TIME [format]");
        }

        if (send(client_socket, response, strlen(response), 0) < 0) {
            perror("Error sending response to client");
            break;
        }
    }

    close(client_socket);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_length;

    // Create server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error creating server socket");
        exit(1);
    }

    // Set server address settings
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(12345);

    // Bind server socket to the specified address and port
    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("Error binding server socket");
        exit(1);
    }

    // Listen for incoming connections
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("Error listening for connections");
        exit(1);
    }

    printf("Time Server is running.\n");

    while (1) {
        client_address_length = sizeof(client_address);

        // Accept client connection
        client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_address_length);
        if (client_socket < 0) {
            perror("Error accepting client connection");
            exit(1);
        }

        printf("Connected by %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

        // Fork a child process to handle the client
        pid_t pid = fork();
        if (pid < 0) {
            perror("Error forking process");
            exit(1);
        }
        else if (pid == 0) {
            // Child process
            close(server_socket);
            handle_client(client_socket);
            exit(0);
        }
        else {
            // Parent process
            close(client_socket);
        }
    }

    close(server_socket);

    return 0;
}
