#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>


void signalHandler(int signo)
{
    int pid = wait(NULL);
    printf("Child %d terminated.\n", pid);
}

int main() 
{
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr))) 
    {
        perror("bind() failed");
        return 1;
    }

    if (listen(listener, 5)) 
    {
        perror("listen() failed");
        return 1;
    }

    signal(SIGCHLD, signalHandler);

    while (1)
    {
        printf("Waiting for new client...\n");
        int client = accept(listener, NULL, NULL);
        if (fork() == 0)
        {
            // Tien trinh con
            close(listener);

            // Xu ly ket noi tu client

            char buffer[256];
            char response[256];
            while (1)
            {
                int ret = recv(client, buffer, sizeof(buffer), 0);
                if (ret <= 0)
                    break;
                buffer[ret - 1] = 0;
                printf("Received from %d: %s\n", client, buffer);

                if (strncmp(buffer, "GET_TIME", 8) == 0) {
                    char* format = buffer + 9;  // Skip "GET_TIME " in the received command'
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

                if (send(client, response, strlen(response), 0) < 0) {
                    perror("Error sending response to client");
                    break;
                }

                // 
            }

                close(client);
                exit(0);
            
        }

        // Tien trinh cha
        close(client);
    }
    
    close(listener);    

    return 0;
}