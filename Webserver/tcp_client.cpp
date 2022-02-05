#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <iostream>

#include<chrono>
#include<thread>

using namespace std;
int main(){


    unsigned short port = 8888;
    char *server_ip = "127.0.0.1";

    int connectfd = socket(AF_INET, SOCK_STREAM,0);

    struct sockaddr_in server_addr;

    bzero(&server_addr, sizeof(server_addr));//填零补齐

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

    int retfd = connect(connectfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if(retfd == -1){
        exit(-1);
    }

    int i = 0;
    while(1){
        char* msg = "weqwe";
        char recvbuf[512];

        sleep(1);

        send(connectfd, msg, 512,0);
        recv(connectfd, recvbuf, sizeof(recvbuf),0);
        printf("recv %s\n",recvbuf);
    }
    close(connectfd);
    return 0;
}
