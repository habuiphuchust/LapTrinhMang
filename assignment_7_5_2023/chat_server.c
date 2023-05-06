#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/select.h>
#include <stdbool.h>

struct client {
    bool check;
    char name[30];
    int id;
};
// Kiểm tra cú pháp
bool checkSyntax(int id, char *syntax) {
    char *i = strstr(syntax, ": ");
    if (i == NULL)
        return false;
    char *token = strtok(syntax, ": ");
    if(atoi(token) != id)
        return false;
    token = strtok(NULL, ": ");
    if (token == NULL)
        return false;
    strcpy(syntax, token);
    return true;
};



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

    fd_set fdread;
    
    struct client clients[64];
    int num_clients = 0;
    
    char buf[256];
    char greeting[64];
    char message[512];

    
    while (1)
    {
        // Xóa tất cả socket trong tập fdread
        FD_ZERO(&fdread);

        // Thêm socket listener vào tập fdread
        FD_SET(listener, &fdread);
        int maxdp = listener + 1;

        // Thêm các socket client vào tập fdread
        for (int i = 0; i < num_clients; i++)
        {
            FD_SET(clients[i].id, &fdread);
            if (maxdp < clients[i].id + 1)
                maxdp = clients[i].id + 1;
        }

        // Chờ đến khi sự kiện xảy ra
        int ret = select(maxdp, &fdread, NULL, NULL, NULL);

        if (ret < 0)
        {
            perror("select() failed");
            return 1;
        }

        // Kiểm tra sự kiện có yêu cầu kết nối
        if (FD_ISSET(listener, &fdread))
        {
            int clientId = accept(listener, NULL, NULL);
            printf("Ket noi moi: %d\n", clientId);
            struct client newClient = {false,"", clientId};
            clients[num_clients++] = newClient;

            // Gửi client hướng dẫn
            sprintf(greeting, "%d: client_name\n", clientId);
            send(clientId, greeting, strlen(greeting), 0);

        }

        // Kiểm tra sự kiện có dữ liệu truyền đến socket client
        for (int i = 0; i < num_clients; i++)
            if (FD_ISSET(clients[i].id, &fdread))
            {
                ret = recv(clients[i].id, buf, sizeof(buf), 0);
                // Nhận ngắt kết nối từ client
                if (ret <= 0)
                {
                    // TODO: Client đã ngắt kết nối, xóa client ra khỏi mảng
                    num_clients--;
                    for (int j = i; j < num_clients; j++)
                        clients[j] = clients[j + 1];
                    i--;
                    continue;
                }

                buf[ret] = 0;

                // Kiểm tra client
                if (!clients[i].check) 
                // Kiểm tra cú pháp lần đầu
                {
                    bool check = checkSyntax(clients[i].id, buf);
                    if (check) 
                    {
                        clients[i].check = true;
                        // Sau khi tokken buf chỉ còn lại tên
                        strcpy(clients[i].name, buf);
                        // xóa xuống dòng
                        clients[i].name[strlen(buf) - 1] = 0;
                        // Gửi client hướng dẫn
                        sprintf(greeting, "\nThanh cong\n");
                        send(clients[i].id, greeting, strlen(greeting), 0);
                    }
                    else
                    {
                        sprintf(greeting, "%d: client_name\n", clients[i].id);
                        send(clients[i].id, greeting, strlen(greeting), 0);
                    }
                }
                else // Gửi dữ liệu đến tất cả client
                {   
                     sprintf(message, "%s: %s", clients[i].name, buf);
                    for (int j = 0; j < num_clients; j++)
                    {   
                        if (clients[j].check)
                            send(clients[j].id, message, strlen(message), 0);
                    }
                }
             
            }
    }

    close(listener);    

    return 0;
}