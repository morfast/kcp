#include<string.h>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <errno.h>

#include "test.h"
#include "ikcp.c"


#define PORT 12345
#define MAXLINE 1024*2

static int sockfd;
static struct sockaddr_in serveraddr;

// kcp call back function
int udp_output(const char *buf, int len, ikcpcb *kcp, void *user)
{
    int n;

    n = sendto(sockfd, buf, len, 0, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if (n >= 0) {
        fprintf(stderr, "%d bytes sent\n", n);
    } else {
        fprintf(stderr, "error: %d bytes send\n", n);
        fprintf(stderr, "%d\n", errno);
        return 1;
    }
	return 0;
}

int main(int argc, char **argv)
{
    char sendline[MAXLINE + 1];
    int n;
    FILE *fp;
    size_t rsize;
    int mode;

    ikcpcb *kcp1 = ikcp_create(0x11223344, (void*)0);
    kcp1->output = udp_output;
    mode = 0;

	IUINT32 current = iclock();
	IUINT32 slap = current + 20;

	if (mode == 0) {
		ikcp_nodelay(kcp1, 0, 10, 0, 0);
	} else if (mode == 1) {
		ikcp_nodelay(kcp1, 0, 10, 0, 1);
	}	else {
		ikcp_nodelay(kcp1, 1, 10, 2, 1);
	}

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
		isleep(8);
		current = iclock();
		ikcp_update(kcp1, iclock());

        rsize = fread(sendline, 1, 1400, fp);
        if (rsize == 0) {
            printf("file read error, or end of file\n");
            break;
        } else {
            printf("%d bytes read from file\n");
        }

        ikcp_send(kcp1, sendline, rsize);
        //n = sendto(sockfd, sendline, 1400, 0, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    }

    return 0;

}
