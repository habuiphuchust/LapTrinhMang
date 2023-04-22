#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
    int receiver = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(atoi(argv[1]));

    bind(receiver, (struct sockaddr *)&addr, sizeof(addr));

    char buf[512];
    struct sockaddr_in sender_addr;
    int sender_addr_len = sizeof(sender_addr);

    //mở file
    FILE *f = fopen("Data.txt", "wb");
    
    // xác nhận udp_sender, mặc định chỉ nhận và lọc file từ sender đầu tiên
    char *ip;
    uint16_t port;
    int i = 0;

    int ret;
    while (1)
    {
        ret = recvfrom(receiver, buf, sizeof(buf), 0, (struct sockaddr *)&sender_addr, &sender_addr_len);
        // vòng lặp đầu tiên, lọc ra sender đầu tiên
        if (i == 0) {
            ip = inet_ntoa(sender_addr.sin_addr);
            port = sender_addr.sin_port;
            i++;
        }

        // xác thực sender
        if ((strcmp(ip, inet_ntoa(sender_addr.sin_addr)) == 0) && (port == sender_addr.sin_port))
        {
            // kiểm tra kí tự kết thúc file
            if (strncmp(buf, "\r\n\r\n\r\n", 6) == 0)
                {
                    fclose(f);
                    break;
                }
            
            fwrite(buf, 1, ret, f);
        }
         printf("số byte nhận: %d IP: %s PORT: %d\n", ret, inet_ntoa(sender_addr.sin_addr), ntohs(sender_addr.sin_port));

    }
    close(receiver);
}