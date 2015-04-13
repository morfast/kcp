#include<string.h>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <errno.h>


#define PORT 12345
#define MAXLINE 1024*2

int main(int argc, char **argv)
{
    int sockfd;
    struct sockaddr_in serveraddr;
    char sendline[MAXLINE + 1];
    int n;
    FILE *fp;
    size_t rsize;

    if (argc != 3) {
        printf("usage: %s <ip address> <filename>\n", argv[0]);
        exit(1);
    }
    strcpy(sendline, "hello,world~, hello, world");

    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(PORT);
    inet_pton(AF_INET, argv[1], &serveraddr.sin_addr);

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        fprintf(stderr, "socket open error\n");
        exit(1);
    }

    // open a file to send
    fp = fopen(argv[2], "r");
    if (fp == NULL) {
        fprintf(stderr, "file open error\n");
        exit(2);
    }


    while (1) {
        rsize = fread(sendline, 1400, 1, fp);
        if (rsize == 0) {
            printf("file read error, or end of file\n");
            break;
        }
        n = sendto(sockfd, sendline, 1400, 0, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
        if (n >= 0) {
            printf("%d bytes sent\n", n);
        } else {
            fprintf(stderr, "error: %d bytes send\n", n);
            fprintf(stderr, "%d\n", errno);
            exit(1);
        }
    }

    return 0;

}
