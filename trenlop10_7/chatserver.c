#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int users[64];
char *id_user[64];
int num_users = 0;

void *client_thread(void *);
int checkId(char*);
void handleMSG(char*, int, int);

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
    int usr;
    // xu ly lenh JOIN <NICKNAME>
    while (1)
    {
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
        {
            close(client);
            return NULL;
        }
        buf[ret] = 0;
        char cmd[16], id[32], tmp[32];
        ret = sscanf(buf, "%s%s%s", cmd, id, tmp);
        id[31] = 0;
        if (ret != 2)
        {
            char *msg = "999 UNKNOWN ERROR\r\n";
            send(client, msg, strlen(msg), 0);
            continue;
        } 
        else 
        {   
            if (strncmp(cmd, "JOIN", 4) == 0)
            {
                if (checkId(id)) {
                    int flag = 1;
                    for (int i = 0; i < num_users; i++)
                    {
                        if(strcmp(id_user[i], id) == 0)
                        {
                            flag = 0;
                            break;
                        }

                    }
                    if (flag) 
                    {
                    // khong trung id
                    char *msg = "100 OK\r\n";
                    send(client, msg, strlen(msg), 0);
                    pthread_mutex_lock(&mutex);
                    users[num_users] = client;
                    id_user[num_users] = id;
                    usr = num_users;
                    num_users++;
                    pthread_mutex_unlock(&mutex);
                    break;
                    }
                    else 
                    {
                        char *msg = "200 NICKNAME IN USE\r\n";
                        send(client, msg, strlen(msg), 0);
                        continue;
                    }
                }
                else
                {
                    char *msg = "201 INVALID NICK NAME\r\n";
                    send(client, msg, strlen(msg), 0);
                    continue;
                }
            }
            else
            {
                char *msg = "999 UNKNOWN ERROR\r\n";
                send(client, msg, strlen(msg), 0);
                continue;
            }
        }
    }
    // dang nhap thanh cong
    while(1)
    {
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
            break;
        buf[ret] = 0;
        puts(buf);
        if (strncmp(buf, "MSG", 3) == 0)
        {
            handleMSG(buf + 4, client, usr);
        }
        else if(strncmp(buf, "PMSG", 4) == 0)
        {

        }
        else if(strncmp(buf, "OP", 2) == 0)
        {

        }
        else if(strncmp(buf, "KICK", 4) == 0)
        {

        }
        else if(strncmp(buf, "TOPIC", 4) == 0)
        {

        }
        else if(strncmp(buf, "QUIT", 4) == 0)
        {

        }

    }

    close(client);
}

// kiem tra ten
int checkId(char* id)
{
    int length = strlen(id);
    for (int i = 0; i < length; i++)
    {
        if ((id[i] < 'a' || id[i] >'z') && (id[i] < '0' || id[i] > '9'))
            return 0;
    }
    return 1;
}

// xu ly MSG
void handleMSG(char* mess, int client, int usr)
{
    pthread_mutex_lock(&mutex);
    char buf[256];
    sprintf(buf,"MSG %s %s", id_user[usr], mess );
    for (int i = 0; i < num_users; i++)
    {
        if (users[i] != client)
        {
            send(users[i], buf, strlen(buf), 0);
        }
    }
    pthread_mutex_unlock(&mutex);
}
