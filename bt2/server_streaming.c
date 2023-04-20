#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>

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
    addr.sin_port = htons(9090);

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

    struct sockaddr_in clientAddr;
    int clientAddrLen = sizeof(clientAddr);

    int client = accept(listener, (struct sockaddr *)&clientAddr, &clientAddrLen);
    if (client == -1)
    {
        perror("accept() failed");
        return 1;
    }
    printf("New client connected: %d\n", client);
    printf("Client IP: %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

    // đếm chuỗi
    char temp[24];
    memset(temp, 'A', 23);
    temp[23] = 0;
    unsigned int count = 0;

    // Truyen nhan du lieu
    char buf[64];
    int ret;
    while (1)
    {
        ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
            break;
        if (ret < sizeof(buf))
            buf[ret] = 0;
        // đếm chuỗi nếu bị ngắt
        strncpy(temp + 9, buf, 9);
        if (strstr(temp, "0123456789") != NULL)
            count++;
        strncpy(temp, buf + sizeof(buf) - 9, 9);
        //đếm chuôi không bị ngắt
        char *pos = buf;
        while (1)
        {
            pos = strstr(pos, "0123456789");
            if (pos == NULL)
                break;
            count++;
            pos += 10;

        }

       
        printf("%s",buf);
    }
    printf("\n%d\n", count);

    close(client);
    close(listener);    

    return 0;
}