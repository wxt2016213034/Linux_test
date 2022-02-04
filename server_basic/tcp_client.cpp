#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
int main(int argc, char *argv[])
{
unsigned short port = 8888;
// unsigned short port = 8000;
// char *server_ip = "8.140.133.32";
char *server_ip = "127.0.0.1";
if(argc > 1)
{
server_ip = argv[1];
}
if(argc > 2)
{
port = atoi(argv[2]);
}
int sockfd = 0;
sockfd = socket(AF_INET, SOCK_STREAM, 0);
if(sockfd < 0)
{
perror("socket ");
exit(-1);
}
struct sockaddr_in server_addr;
bzero(&server_addr, sizeof(server_addr));
server_addr.sin_family = AF_INET;
server_addr.sin_port = htons(port);
inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

int err_log = connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
if(err_log != 0)
{
perror("connect");
close(sockfd);
exit(-1);
}
while(1)
{
char send_buf[512] = " ";
char recv_buf[512] = " ";
printf("send data to %s:%d\n",server_ip, port);

printf("send:");
fgets(send_buf, sizeof(send_buf), stdin);
send_buf[strlen(send_buf)-1] = 0;
send(sockfd, send_buf, strlen(send_buf), 0);//向服务器发送数据
printf("send:%s\n",send_buf);

recv(sockfd, recv_buf, sizeof(recv_buf), 0);//接收数据
printf("recv:%s\n",recv_buf);


}


return 0;
}