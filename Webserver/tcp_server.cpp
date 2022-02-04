#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <iostream>
#include <errno.h>
#include <signal.h>
void recycleChild(int arg){
    while(1){//标识位只能被设置一次
        int ret = waitpid(-1,NULL,WNOHANG);
        if(ret == -1 || ret == 0)break;
        else std::cout<< "回收了"<<ret<<std::endl;
    }
}

int main(){

    struct sigaction act;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    act.sa_handler = recycleChild;
    sigaction(SIGCHLD,&act,NULL);
    
    //创建socket
    int lfd = socket(PF_INET, SOCK_STREAM,0);

    if(lfd == -1){
        perror("socket");
        exit(-1);
    }

    //服务器addr
    struct sockaddr_in saddr;
    
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(8888);
    saddr.sin_addr.s_addr = INADDR_ANY;

    //bind
    int ret = bind(lfd, (struct sockaddr *)&saddr, sizeof(saddr));
    if(ret == -1){
        perror("bind");
        exit(0);
    }

    // listen
    ret = listen(lfd,128);
    if(ret == -1){
        perror("listen");
        exit(-1);
    }

    //循环等待
    while(1){

        struct sockaddr_in cliaddr;
        socklen_t len = sizeof(cliaddr);


        //接受链接
        int cfd = accept(lfd, (struct sockaddr*)&cliaddr,&len);
        if(cfd == -1){
            if(errno == EINTR){
                continue;
            }//处理在accept时，进行系统调用，accept不再阻塞而记录一个errno为EINTR，不处理会导致主程序退出
            perror("accept");
            exit(-1);
        }

        pid_t pid = fork();
        if(pid == 0){
            //子进程
            char cliIp[16];
            inet_ntop(AF_INET, &cliaddr.sin_addr.s_addr,cliIp,sizeof(cliIp));
            unsigned short cliPort = ntohs(cliaddr.sin_port);

            printf("client Ip is %s, port is %d\n",cliIp,cliPort);

            //接受客户端数据
            char recvBuf[1024];
            while(1){
                int len = recv(cfd,(void *)&recvBuf,sizeof(recvBuf),0);

                if(len == -1){
                    perror("read");
                    exit(-1);
                }else if(len >0){
                    printf("recv from client data is %s\n", recvBuf);
                }else{
                    printf("client close....");
                    break;
                }

                write(cfd, recvBuf, strlen(recvBuf));
            }

            close(cfd);
            exit(0);
        }

    }

    close(lfd);

    return 0;
}