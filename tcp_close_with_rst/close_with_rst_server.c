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
    int opt, ret = 0,reuse = 1;
    int option_index;
    static struct option lgopts[] =
    {
        {NULL, 0, 0, 0}
    };
    unsigned int lis_port = 0;
    char host_ip[16] = {0};
	struct sockaddr_in local_addr, peer_addr;
	int listen_fd = 0, sockfd= 0;
	int addrlen = sizeof(struct sockaddr);
    char *buff = NULL;
    int buflen = 128 * 1024;  //default 128k

    /* 解析参数 */
    if (argc == 1)
    {
        printf("Usage:\n %s -h [local-host-ip] -p [local-port] -s [send Bytes]\t\t \n", argv[0]);
        return -1;
    }

    while ((opt = getopt_long(argc, argv, "h:p:s:",
                  lgopts, &option_index)) != EOF)
    {
        switch (opt)
        {
            case 'p':				
                lis_port = atoi(optarg);
                break;
            case 's':				
                buflen = atoi(optarg);
                break;
            case 'h':				
                strncpy (host_ip, optarg, sizeof(host_ip) - 1);
                break;
            default:
                printf("Usage:\n %s -h [host-ip] -p [port]\t\t\n", argv[0]);
				return -1;
		}
    }

    buff = malloc(buflen);
    if (buff == NULL)
    {
        printf("malloc %d Bytes send buffer failed\n", buflen);
        return -1;
    }
   
    /* 监听socket的参数*/
 	bzero(&local_addr, sizeof(local_addr));
	local_addr.sin_family 		= PF_INET;
	local_addr.sin_port 		= htons(lis_port);
	inet_aton(host_ip, &local_addr.sin_addr);   

    /*建立socket*/
    listen_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1)
    {
        perror("socket create");
        return -1;
    }

    /*绑定地址端口*/
	setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
   	ret = bind(listen_fd, (struct sockaddr *)&local_addr, sizeof(local_addr));
	if (ret == -1)
	{
		perror("bind");
		return -1;
	}		

    ret = listen(listen_fd, 5);
    if (ret == -1)
    {
        perror("listen error");
        return -1;
    }

    while(1)
    {
        /* 接收连接*/
        sockfd = accept(listen_fd, (struct sockaddr *)&peer_addr, (socklen_t*)&addrlen);
        if (sockfd == -1)
        {
            printf("accept error");
            continue;
        }

        printf("host %s:%d connected in\n", inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port));

        printf("send %d Bytes packet to host %s:%d\n", buflen, inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port));
    	ret = send(sockfd, buff, buflen, 0);

//        close(sockfd);
    }

    return 0;
}



