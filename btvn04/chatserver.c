#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>

int users[64];      // Mang socket client da dang nhap
char *user_ids[64]; // Mang id client da dang nhap
int num_users = 0;  // So luong client da dang nhap
pthread_mutex_t user_mutex = PTHREAD_MUTEX_INITIALIZER;

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

    ///// kiem tra dang nhap
    int login = 0;
    int j = 0;
    for (; j < num_users; j++)
        if (users[j] == client)
        {
            login = 1;
            break;
        }

    
    while (1)
    {
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
        {
            if (login)  // xoa user khoi mang
            {
                pthread_mutex_lock(&user_mutex);
                num_users--;
                j = 0;
                for (; j < num_users; j++)
                        if (users[j] == client)
                            break;
                users[j] = users[num_users];
                user_ids[j] = user_ids[num_users];
                printf("num client: %d\n",num_users);
                pthread_mutex_unlock(&user_mutex);
                
            }
            break;
        }
        
        buf[ret] = 0;
        printf("Received from %d: %s\n", client, buf);
        //
          
        if (!login) // Chua dang nhap
        {
            // Xu ly cu phap lenh dang nhap
            char cmd[32], id[32], tmp[32];
            ret = sscanf(buf, "%s%s%s", cmd, id, tmp);
            if (ret == 2)
            {
                if (strcmp(cmd, "client_id:") == 0)
                {
                    char *msg = "Dung cu phap. Hay nhap tin nhan de chuyen tiep.\n";
                    send(client, msg, strlen(msg), 0);
                    // Luu vao mang user
                    pthread_mutex_lock(&user_mutex);
                    users[num_users] = client;
                    user_ids[num_users] = malloc(strlen(id) + 1);
                    strcpy(user_ids[num_users], id);
                    j = num_users;
                    num_users++;
                    login = 1;
                    pthread_mutex_unlock(&user_mutex);

                }
                else
                {
                    char *msg = "Sai cu phap. Hay nhap lai.\n";
                    send(client, msg, strlen(msg), 0);
                }
            }
            else
            {
                char *msg = "Sai tham so. Hay nhap lai.\n";
                send(client, msg, strlen(msg), 0);
            }
        }
        else // Da dang nhap
        {
            // id: user_ids[j]
            // data: buf

            char sendbuf[256];

            strcpy(sendbuf, user_ids[j]);
            strcat(sendbuf, ": ");
            strcat(sendbuf, buf);

            // Forward du lieu cho cac user
            for (int k = 0; k < num_users; k++)
                if (users[k] != client)
                    send(users[k], sendbuf, strlen(sendbuf), 0);
        }
/////
    }

    close(client);
}