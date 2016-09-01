#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/queue.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/stat.h>
#include <assert.h>
#include <syslog.h>
#include <sys/socket.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <signal.h>


int main(int argc, char ** argv)
{
    int opt, ret = 0;
    int option_index, sleep_sec = 0;
    static struct option lgopts[] =
    {
        {NULL, 0, 0, 0}
    };
    unsigned int lis_port = 0;
    char host_ip[16] = {0};
	struct sockaddr_in peer_addr;
	int sockfd= 0;
	int addrlen = sizeof(struct sockaddr);

    /* 解析参数 */
    if (argc == 1)
    {
        printf("Usage:\n %s -h [remote-host-ip] -p [remote-port] -d [delay seconds before close]\t\t \n", argv[0]);
        return -1;
    }

    while ((opt = getopt_long(argc, argv, "h:p:d:",
                  lgopts, &option_index)) != EOF)
    {
        switch (opt)
        {
            case 'p':				
                lis_port = atoi(optarg);
                break;
            case 'h':				
                strncpy (host_ip, optarg, sizeof(host_ip) - 1);
                break;
            case 'd':				
                sleep_sec = atoi(optarg);
                break;
            default:
                printf("Usage:\n %s -h [remote-host-ip] -p [remote-port] -d [delay seconds before close]\t\t \n", argv[0]);
				return -1;
		}
    }

    /* 远端socket的参数*/
 	bzero(&peer_addr, sizeof(peer_addr));
	peer_addr.sin_family 		= PF_INET;
	peer_addr.sin_port 		= htons(lis_port);
	inet_aton(host_ip, &peer_addr.sin_addr);   

    /*建立socket*/
    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket create");
        return -1;
    }

    printf("connect to host %s:%d\n", inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port));
    /* 连接*/
    ret = connect(sockfd, (struct sockaddr *)&peer_addr, addrlen);
    if (ret != 0)
    {
        printf("connect error, ret=%d\n", sockfd);
        return -1;
    }

    printf("connect to host %s:%d succeed, sleep %d seconds \n", 
        inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port), sleep_sec);
		
	sleep(sleep_sec);
    close(sockfd);
    printf("close connect with host %s:%d\n", inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port));

    return 0;
}



