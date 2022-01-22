#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
int main(int argc, char *argv[])
{
unsigned short port = 8000;
if(argc > 1)
{
port = atoi(argv[1]);
}
int sockfd = socket(AF_INET, SOCK_STREAM, 0);
if(sockfd < 0)
{
perror("sockfd");
exit(-1);
}
struct sockaddr_in my_addr;//本地信息
bzero(&my_addr, sizeof(my_addr));
my_addr.sin_family = AF_INET;//设置地址族为IPv4
my_addr.sin_port = htons(port);//设置地址的端口号信息
my_addr.sin_addr.s_addr = htonl(INADDR_ANY);//INADDR_ANY服务器可以接收任意地址

int err_log = bind(sockfd, (struct sockaddr*)&my_addr, sizeof(my_addr));
if(err_log != 0)
{
perror("binding");
close(sockfd);
exit(-1);
}

err_log = listen(sockfd, 10);
if(err_log != 0)
{
perror("listen");
close(sockfd);
exit(-1);
}
printf("listen client %d\n",port); 

struct sockaddr_in client_addr;
char cli_ip[INET_ADDRSTRLEN] = " ";
socklen_t client_len = sizeof(client_addr);
int connfd;
connfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_len);//等待连接
if(connfd < 0 )
{
perror("accept");
return -1;
//continue;
}
inet_ntop(AF_INET, &client_addr.sin_addr, cli_ip, INET_ADDRSTRLEN);//转换并打印信息
printf("********************************\n");
while(1)
{
char recv_buf[2048] = " ";
recv(connfd, recv_buf, sizeof(recv_buf), 0);//接收信息
printf("recv data:%s\n",recv_buf);
send(connfd, "ok", 2, 0);//向客户端发送数据
printf("ok\n");
}
close(sockfd);//关闭监听套接字
return 0;
}