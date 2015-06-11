//
// TCP 回射服务器
//
// 实现源自《Unix 网络编程（卷1）》6.8 小节。同 tcpselect.c，自己实现一遍，
// 主要还是熟悉下 select 函数的使用。
// 
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include "IOHelper.h"

#define SERV_PROT 49233
#define LISTENNQ 5000
#define MAXLINE 500

int main(int argc, char const *argv[])
{
    int listenfd;   // 监听套接字
    struct sockaddr_in servaddr;
    int maxfd;
    int maxi;
    int client[FD_SETSIZE];
    fd_set allset;
    int i;

    fd_set rset;
    int nready; // 可读的描述符的个数
    socklen_t clilen;
    struct sockaddr_in cliaddr;
    int connfd;

    int sockfd;

    char buf[MAXLINE];
    int n;

    int sucess;

    char address[50];
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PROT);
    sucess = bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if (sucess == -1)
    {
        printf("bind failed\n");
        return 1;
    }

    sucess = listen(listenfd, LISTENNQ);
    if (sucess == -1)
    {
        printf("listen failed\n");
        return 1;
    }
    printf("listen sucessed\n");

    maxfd = listenfd;
    maxi = -1;
    for (i = 0; i < FD_SETSIZE; ++i)
    {
        client[i] = -1;
    }
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);
    while(1)
    {
        rset = allset;
        printf("call select\n");
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
        if (nready == -1)
        {
            printf("select error\n");
            return 1;
        }
        printf("select sucessed. return value: %d\n", nready);

        if (FD_ISSET(listenfd, &rset))
        {
            // 监听套接字可读
            clilen = sizeof(cliaddr);
            connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
            bzero(address, sizeof(address));
            inet_ntop(AF_INET, &cliaddr.sin_addr, address, sizeof(address));
            printf( "new client: %s, port %d\n", 
                    address,
                    ntohs(cliaddr.sin_port));

            for (i = 0; i < FD_SETSIZE; ++i)
            {
                if (client[i] < 0)
                {
                    client[i] = connfd;
                    break;
                }
            }
            if (i == FD_SETSIZE)
            {
                printf("too many clients\n");
                close(connfd);
                continue;
            }
            FD_SET(connfd, &allset);

            maxfd = max(maxfd, connfd);
            maxi = max(maxi, i);

            if (--nready <= 0)
            {
                // 说明只有监听套接字可读
                continue;
            }
        }

        for (i = 0; i <= maxi; ++i)
        {
            sockfd = client[i];
            if (sockfd < 0)
            {
                continue;
            }

            if (FD_ISSET(sockfd, &rset))
            {
                bzero(buf, sizeof(buf));
                n = read(sockfd, buf, sizeof(buf));
                if (n == 0 || n < 0)
                {
                    printf("read finished or failed. return value: %d\n", n);
                    close(sockfd);
                    FD_CLR(sockfd, &allset);
                    client[i] = -1;

                    // TODO(H): 我觉得在这里应该重新计算下要要传给 select 的第一个参数的值。
                    //          因为当前被关闭的这个套接字很有可能就是传给 select 的描述符集中值最大的描述符。
                    //          但这里没有计算，不知道为什么？说实话，我们对 select 函数的第一个参数的理解并不够
                }
                else
                {
                    sucess = write(sockfd, buf, n);
                    if (sucess == -1 || sucess == 0)
                    {
                        printf("write failed or write return 0. return value: %d\n", sucess);
                        return 1;
                    }
                }

                if (--nready <= 0)
                {
                    break;
                }
            }
        }
    }

    return 0;
}