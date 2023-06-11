#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
FILE *f;
pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;

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
    short int login = 0;
    while (1)
    {
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
            break;
        buf[ret] = 0;
        printf("Received from %d: %s\n", client, buf);

    
        if (!login)
        {
            // Chua dang nhap
            char user[32], pass[32], tmp[65], line[65];
            int ret = sscanf(buf, "%s%s%s", user, pass, tmp);
            if (ret == 2)
            {
                int found = 0;
                sprintf(tmp, "%s %s\n", user, pass);
                FILE *f1 = fopen("users.txt", "r");
                while (fgets(line, sizeof(line), f1) != NULL)
                {
                    if (strcmp(line, tmp) == 0)
                    {
                        found = 1;
                        login = 1;
                        break;
                    }                    
                }
                fclose(f1);

                if (found)
                {
                    char *msg = "Dang nhap thanh cong. Hay nhap lenh de thuc hien.\n";
                    send(client, msg, strlen(msg), 0);
                }
                else
                {
                    char *msg = "Nhap sai tai khoan. Hay nhap lai.\n";
                    send(client, msg, strlen(msg), 0);
                }
            }
            else
            {
                char *msg = "Nhap sai cu phap. Hay nhap lai.\n";
                send(client, msg, strlen(msg), 0);
            }
        }
        else
        {
            // Da dang nhap
            char tmp[276];
            if (buf[strlen(buf) - 1] == '\n')
                buf[strlen(buf) - 1] = '\0';
            sprintf(tmp, "%s > out.txt", buf);

            // Thuc hien lenh
            system(tmp);

            // Tra ket qua cho client
            pthread_mutex_lock(&file_mutex);
            f = fopen("out.txt", "rb");
            while (!feof(f))
            {
                int ret = fread(tmp, 1, sizeof(tmp), f);
                if (ret <= 0)
                    break;
                send(client, tmp, ret, 0);
            }
            fclose(f);
            pthread_mutex_unlock(&file_mutex);
        }
    }

    close(client);
}