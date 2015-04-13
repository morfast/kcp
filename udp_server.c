#include<string.h>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>

#include "test.h"
#include "ikcp.c"

#define PORT 12345
#define MAXLINE 10240

int udp_output(const char *buf, int len, ikcpcb *kcp, void *user)
{
	return 0;
}

int main(void)
{
    int sockfd;
    int err;
    struct sockaddr_in serveraddr, cliaddr;
    int n;
    socklen_t len;
    char mesg[MAXLINE];
    int npacket;

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
        isleep(8);
		ikcp_update(kcp1, iclock());

        while (1) {
            n = ikcp_recv(kcp1, mesg, n);
            if (n < 0) break;
            printf("%d received from ikcp_recv\n", n);
            printf("%s\n", mesg);
        }
        while (1) {
            isleep(8);
            ikcp_update(kcp1, iclock());
            n = recvfrom(sockfd, mesg, MAXLINE, 0, (struct sockaddr *)&cliaddr, &len);
            if (n < 0) {
                break;
            }
            ikcp_input(kcp1, mesg, n);
            printf("%d received from recvfrom\n", n);
            printf("%s\n", mesg);
        }
    }

    return 0;

}

