#include<string.h>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include <errno.h>

#include "test.h"
#include "ikcp.c"

#define PORT 12345
#define MAXLINE 10240

static int sockfd;
static struct sockaddr_in serveraddr, cliaddr;

int udp_output(const char *buf, int len, ikcpcb *kcp, void *user)
{
    int n;

    n = sendto(sockfd, buf, len, 0, (struct sockaddr *)&cliaddr, sizeof(cliaddr));
    if (n >= 0) {
        fprintf(stderr, "%d bytes sent: ", n);
        fprintf(stderr, "%s\n", buf);
    } else {
        fprintf(stderr, "error: %d bytes send\n", n);
        fprintf(stderr, "%d\n", errno);
        return 1;
    }
	return 0;
}

void print_msg(char *str, int len)
{
    int i;

    for (i = 0; i < len; i++) {
        printf("%c", str[i]);
    }
    printf("\n");
}

int main(void)
{
    int err;
    int n;
    socklen_t len;
    char mesg[MAXLINE];
    char mesg2[MAXLINE];
    int npacket;
    FILE *fp;

    fp = fopen("output.txt", "wb");
    int fd = creat("output",0755);
    ikcpcb *kcp1 = ikcp_create(0x11223344, (void*)1);
    kcp1->output = udp_output;
    int mode = 0;

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
        isleep(1);
		ikcp_update(kcp1, iclock());
        n = recvfrom(sockfd, mesg, MAXLINE, 0, (struct sockaddr *)&cliaddr, &len);
        err = ikcp_input(kcp1, mesg, n);
        if (err < 0) {
            fprintf(stderr, "ikcp_input error: %d\n", err);
        }
        //printf("%d received from recvfrom\n", n);
        //print_msg(mesg, n);
        while (1) {
            //isleep(1);
            //ikcp_update(kcp1, iclock());
            n = ikcp_recv(kcp1, mesg2, MAXLINE);
            if (n > 0) {
                write(fd, mesg2, n);
            }
            if (n < 0) break;
            printf("%d received from ikcp_recv\n", n);
//            print_msg(mesg2, n);
        }
    }
	ikcp_release(kcp1);
    fclose(fp);
    close(fd);

    return 0;

}

