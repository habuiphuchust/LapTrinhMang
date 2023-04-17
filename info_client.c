#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

void clean_stdin()
{
    int c;
    do {
        c = getchar();
    } while (c != '\n' && c != EOF);
}

int main() {
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(9000); 

    if (connect(client, (struct sockaddr *)&addr, sizeof(addr))) {
        perror("connect() failed");
        return 1;
    }
        
    // Truyen nhan du lieu
    char ten[32];
    char tenodia[2];
    char odias[128];
    int soluong;
    int bonho;
    char buf[256];

    printf("Nhap thong tin cua may tinh:\n");
    printf("Nhap ten may tinh: ");
    scanf("%s", ten);
    clean_stdin();

    printf("nhap so o dia: ");
    scanf("%d", &soluong);

    char *point = buf;
    sprintf(buf,"%s\n%d\n", ten, soluong);
    point = buf + strlen(buf);

    for (int i = 0; i < soluong; i++) {
        printf("nhap ten o dia : ");
        clean_stdin;
        scanf("%s", tenodia);
        printf("nhap dung luong: ");
        scanf("%d", &bonho);
        sprintf(point, "%s - %d\n", tenodia, bonho);
        point = point + strlen(point);


    }
    printf("%s", buf);
    
    send(client, buf, strlen(buf), 0);

    // Ket thuc, dong socket
    close(client);

    return 0;
}