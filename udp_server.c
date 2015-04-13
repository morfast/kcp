#include<string.h>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>

#define PORT 12345
#define MAXLINE 10240

int main(void)
{
    int sockfd;
    int err;
    struct sockaddr_in serveraddr, cliaddr;
    int n;
    socklen_t len;
    char mesg[MAXLINE];
    int npacket;

    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(PORT);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        fprintf(stderr, "socket of server error\n");
        exit(1);
    }

    err = bind(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
    if (err < 0) {
        fprintf(stderr, "socket of server error\n");
        exit(1);
    }

    npacket = 0;
    while (1) {
        n = recvfrom(sockfd, mesg, MAXLINE, 0, (struct sockaddr *)&cliaddr, &len);
        if (n < 0) {
            fprintf(stderr, "recvfrom error\n");
            exit(1);
        }
        npacket++;
        printf("%d received, %d packets\n", n, npacket);
        if (npacket > 100000) break;
    //    printf("%s\n", mesg);
    }

    return 0;

}

