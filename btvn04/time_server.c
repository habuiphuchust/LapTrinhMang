#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

void *client_thread(void *);

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

    while (1)
    {
        int client = accept(listener, NULL, NULL);
        if (client == -1)
        {
            perror("accept() failed");
            continue;;
        }
        printf("New client connected: %d\n", client);

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, client_thread, &client);
        pthread_detach(thread_id);
    }
    
    close(listener);    

    return 0;
}

void *client_thread(void *param)
{
    int client = *(int *)param;
    char buf[256];
    char response[256];
    while (1)
    {
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
            break;
        buf[ret - 1] = 0;
        printf("Received from %d: %s\n", client, buf);

        if (strncmp(buf, "GET_TIME", 8) == 0) 
        {
            char* format = buf + 9;  // Skip "GET_TIME " in the received command'
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
}