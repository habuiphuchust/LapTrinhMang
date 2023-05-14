#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <poll.h>

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

    struct pollfd fds[64];
    int nfds = 1;

    fds[0].fd = listener;
    fds[0].events = POLLIN;

    char buf[256];
    char userPw[64];

    int users[64];      // Mang socket client da dang nhap
    int num_users = 0;  // So luong client da dang nhap

    FILE *f;
    f = fopen("csdl.txt", "r");


    while (1)
    {
        int ret = poll(fds, nfds, -1);
        if (ret < 0)
        {
            perror("poll() failed");
            break;
        }

        if (fds[0].revents & POLLIN)
        {
            int client = accept(listener, NULL, NULL);
            if (nfds == 64)
            {
                // Tu choi ket noi
                close(client);
            }
            else
            {
                fds[nfds].fd = client;
                fds[nfds].events = POLLIN;
                nfds++;

                printf("New client connected: %d\n", client);
            }
        }

        for (int i = 1; i < nfds; i++)
            if (fds[i].revents & POLLIN)
            {
                ret = recv(fds[i].fd, buf, sizeof(buf), 0);
                if (ret <= 0)
                {
                    close(fds[i].fd);
                    // Xoa khoi mang
                    if (i < nfds - 1)
                        fds[i] = fds[nfds - 1];
                    nfds--;
                    i--;
                }
                else
                {
                    buf[ret] = 0;
                    printf("Received from %d: %s\n", fds[i].fd, buf);
                    //
                     int client = fds[i].fd;

                        // Kiem tra trang thai dang nhap
                        int j = 0;
                        for (; j < num_users; j++)
                            if (users[j] == client)
                                break;
                        
                        if (j == num_users) // Chua dang nhap
                        {
                            // Xu ly cu phap lenh dang nhap
                            char cmd[32], id[32], tmp[32];
                            ret = sscanf(buf, "%s%s%s", cmd, id, tmp);
                            if (ret == 2)
                            {
                                int flag = 0;
                                while (fgets(userPw, sizeof(userPw), f)) {
                                    if (strcmp(buf, userPw) == 0) {
                                        char *msg = "Dang nhap thanh cong. Moi nhap lenh\n";
                                        send(client, msg, strlen(msg), 0);

                                        // Luu vao mang user
                                        users[num_users] = client;
                                        num_users++;
                                        flag = 1;
                                        break;
                                    }
                                }
                                // ko co trong mang user da dang nhap
                                if (!flag) {
                                    char *msg = "Sai tai khoan hoac mat khau.\n";
                                    send(client, msg, strlen(msg), 0);
                                }
                                // chuyen con tro file ve dau file
                                fseek(f, 0, SEEK_SET);

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

                            char cmd[256];

                            // xoa ki tu xuong dong
                            char *snew;
                            if (snew = strchr(buf, '\n'))
                                snew[0] = 0;

                            strcpy(cmd, buf);
                            strcat(cmd, " > output.txt");
                            system(cmd);

                            // gui ket qua cho user
                            FILE *result = fopen("output.txt", "r");
                            int ret = 0;
                            while(!feof(result)) {
                                ret = fread(buf, 1, sizeof(buf), result);
                                if (ret <= 0)
                                    break;
                                send(client, buf, ret, 0);    
                            }
                            fclose(result);
                        }
                    //
                }
            }
    }
    
    close(listener); 
    fclose(f);   

    return 0;
}