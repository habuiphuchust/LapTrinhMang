#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char * argv[])
{
    int sender = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    // thiết lập địa chỉ ip + cổng từ dòng lệnh
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[2]);
    addr.sin_port = htons(atoi(argv[3]));

    char *filename = argv[1];
    
    int name_size = strlen(filename);

    FILE *f = fopen(filename, "rb");
    
    if (f == NULL)
    {
        printf("open file %s failed", filename);
        return 1;
    }

    char buf[32];

    // truyền tên file
    strcpy(buf, filename);
    strcpy(buf + name_size, "\r\n\r\n");
    int ret = sendto(sender, buf, name_size + 4, 0, (struct sockaddr *)&addr, sizeof(addr));

    //truyền dữ liệu
     while (!feof(f))
    {
        ret = fread(buf, 1, sizeof(buf), f);
        if (ret <= 0)
            break;
        sendto(sender, buf, ret, 0, (struct sockaddr *)&addr, sizeof(addr));  
        sleep(4);
    }
    // đóng file
    fclose(f);

    // gửi server báo kết thúc file
    strcpy(buf, "\r\n\r\n\r\n");
    sendto(sender, buf, 6, 0, (struct sockaddr *)&addr, sizeof(addr));

    //đóng socket
    close(sender);
}