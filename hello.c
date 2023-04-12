#include<stdio.h>
int main(int argc, char const *argv[])
{
    char s[20];
    fgets(s, sizeof(s), stdin);
    for (int i = 0; i < 20; i++)
    {
        printf("%c", s[i]);
        if (s[i] == '\n') 
            printf("%d", i);
    }
    return 0;
}
