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
#define UDP_PACKET_SIZE 900

static int sockfd;
static struct sockaddr_in serveraddr, cliaddr;

void print_msg(char *str, int len)
{
    int i;

    for (i = 0; i < len; i++) {
        printf("%c", str[i]);
    }
    printf("\n");
}

// kcp call back function
int udp_output(const char *buf, int len, ikcpcb *kcp, void *user)
{
    int n;

    n = sendto(sockfd, buf, len, 0, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if (n >= 0) {
        fprintf(stderr, "%d bytes sent\n", n);
        //print_msg(buf, len);
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
    int ret;
    socklen_t len;


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

	ikcp_wndsize(kcp1, 1024, 1024);

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


    int file_end = 0;
    while (1) {
		isleep(1);
        fprintf(stderr, "ikcp_update\n");
		ikcp_update(kcp1, iclock());

        rsize = fread(sendline, 1, UDP_PACKET_SIZE, fp);
        if (rsize == 0) {
            printf("end of file\n");
            file_end = 1;
        } else {
            printf("%d bytes read from file\n", (int)rsize);
            ret = ikcp_send(kcp1, sendline, rsize);
            isleep(1);
            ikcp_update(kcp1, iclock());
            if (ret < 0) {
                fprintf(stderr, "ikcp_send error\n");
                exit(1);
            } else {
                fprintf(stdout, "ikcp_send success\n");
            }
        }

        while (1) {
            //n = recvfrom(sockfd, sendline, MAXLINE, MSG_DONTWAIT, (struct sockaddr *)&cliaddr, &len);
            n = recvfrom(sockfd, sendline, MAXLINE, MSG_DONTWAIT, NULL, NULL);
            if (n < 0) {
                break;
            }
            printf("%d received from recvfrom: ", n);
            printf("%s\n", sendline);
            ikcp_input(kcp1, sendline, n);
            isleep(1);
            ikcp_update(kcp1, iclock());
        }

        while(1) {
            n = ikcp_recv(kcp1, sendline, MAXLINE);
            isleep(1);
            ikcp_update(kcp1, iclock());
            if (n < 0) break;
            printf("ikcp_recv\n");
        }

        if (file_end) break;

    }
	ikcp_release(kcp1);

    return 0;

}
