// 
// TCP 回射服务器的客户端。
// 
// 实现源自《Unix 网络编程（卷2）》 6.7 小节。自己实现一遍主要是为了熟悉 select 函数的使用，
// 另外这个程序中涉及到对 TCP 半关闭特性的使用和批量输入的处理也值得我们学习。
// 
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <unistd.h>
#include <errno.h>

#include "IOHelper.h"

#define SERVER_PORT 49233
// #define SERVER_PORT 7
#define MAXLINE 500


void str_cli(FILE *fp, int sockfd);

int main(int argc, char const *argv[])
{
    int sockfd;
    struct sockaddr_in servaddr;
    int sucess;

    if (argc != 2)
    {
        printf("usage: tcpselectcli <IP>\n");
        return 1;
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    sucess = connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if (sucess == -1)
    {
        printf("connect failed\n");
        return 1;
    }
    printf("connect sucessed\n");

    str_cli(stdin, sockfd);

    return 0;
}

void str_cli(FILE *fp, int sockfd)
{
    int maxfdp1;    // select 的第一个参数
    int stdineof;   // 表示数据输入是否完成
    fd_set rset;
    char buf[MAXLINE];
    int n;

    int sucess;

    stdineof = 0;
    while(1)
    {
        // Step 1: 设置要传递给 select 的描述符集
        FD_ZERO(&rset);
        FD_SET(sockfd, &rset);
        if (stdineof == 0)
        {
            FD_SET(fileno(fp), &rset);
            maxfdp1 = max(fileno(fp), sockfd) + 1;
        }
        else
        {
            maxfdp1 = sockfd + 1;
        }

        printf("call select\n");
        sucess = select(maxfdp1, &rset, NULL, NULL, NULL);
        if (sucess == -1)
        {
            printf("select error\n");
            return;
        }
        printf("select sucessed. return value: %d\n", sucess);

        if (FD_ISSET(sockfd, &rset))
        {
            // 套接字可读
            bzero(buf, sizeof(buf));
            n = read(sockfd, buf, sizeof(buf));
            if (n == 0)
            {
                // 已收到对端的 FIN 分节
                if (stdineof != 1)
                {
                    // 数据输入尚未完成，却收到对端的 FIN 分节，说明对端异常终止了，比如对端主机关机了
                    printf("str_cli: server terminated prematurely\n");
                }
                return;
            }
            else if (n == -1)
            {
                // TODO: 读数据出现错误，原因多种多样，比如收到了 RST 分节，额，还可能有其他原因吗？这个可以考虑下
                printf("str_cli: read error. error message: %s\n", strerror(errno));
                return;
            }
            else
            {
                // 读取数据成功，将其写到标准输出
                write(fileno(stdout), buf, n);
            }
        }

        if (stdineof == 0 && FD_ISSET(fileno(fp), &rset))
        {
            // 标准输入可读
            bzero(buf, sizeof(buf));
            n = read(fileno(stdin), buf, sizeof(buf));
            if (n == 0)
            {
                // 读入了 EOF，表示数据输入完成，但因为现在对端的数据我们可能还没有完全接受到，
                // 所以这里调用 shutdown 使用 TCP 的半关闭特性
                printf("read data from stdin finished\n");
                stdineof = 1;
                shutdown(sockfd, SHUT_WR);
                FD_CLR(sockfd, &rset);
                continue;
            }
            else if (n > 0)
            {
                // 读入数据成功，写入套接字
                //
                // When using non-blocking I/O on objects, such as sockets, that are subject to flow control, 
                // write() and writev() may write fewer bytes than requested; the return value must be noted, 
                // and the remainder of the operation should be retried when possible.
                sucess = write(sockfd, buf, n);
                if (sucess == -1 || sucess == 0)
                {
                    printf("write failed or write return 0. return value: %d\n", sucess);
                    return;
                }
            }
            else
            {
                // 读入数据失败
                return;
            }
        }
    }
}

