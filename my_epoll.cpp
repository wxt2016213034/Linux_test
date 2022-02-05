#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <sys/epoll.h>

int main(){

    int fd = socket(AF_INET,SOCK_STREAM,0);

    struct sockaddr_in server_addr;
    server_addr.sin_port = htons(8888);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = 0;

    bind(fd, (struct sockaddr*)&server_addr, sizeof(server_addr));

    int num_fd = 5;

    listen(fd, num_fd);

    int epfd = epoll_create(100);

    struct epoll_event epev;
    epev.events = EPOLLIN;
    epev.data.fd = fd;
    epoll_ctl(epfd,EPOLL_CTL_ADD, fd, &epev);

    struct epoll_event epevs[1024];

    // struct sockaddr_in client_addr;
    // int size_client = sizeof(client_addr);
    // int client_fd = accept(fd, (struct sockaddr*)&client_addr,(socklen_t *)&size_client);

    while(1){

        int ret = epoll_wait(epfd,epevs,1024,-1);

        if(ret == -1){
            perror("epoll_wait");
            exit(-1);
        }

        for(int i = 0; i < ret; ++i){
            int curfd = epevs[i].data.fd;

            if(curfd == fd){
                struct sockaddr_in client_addr;
                int size_client = sizeof(client_addr);
                int client_fd = accept(fd, (struct sockaddr*)&client_addr,(socklen_t *)&size_client);
                epev.events = EPOLLIN;
                epev.data.fd = client_fd;
                epoll_ctl(epfd,EPOLL_CTL_ADD, client_fd, &epev);
            }else{
                if(epevs[i].events & EPOLLOUT){
                    continue;
                }
                char buf[512];
                int len = read(curfd,(void *)&buf,sizeof(buf));
                if(len == -1){
                    perror("read");
                    exit(-1);
                }else if(len == 0){
                    std::cout<<"close.."<<std::endl;
                    epoll_ctl(epfd,EPOLL_CTL_DEL,curfd,NULL);
                    close(curfd);
                }else{
                    std::cout<<buf<<std::endl;
                    send(curfd,(void *)buf,sizeof(buf),0);
                }

            }

        }

    }
    close(fd);
    close(epfd);

}