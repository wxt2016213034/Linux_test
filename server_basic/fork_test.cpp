#include<iostream>
#include <sys/types.h>
#include <unistd.h>
using namespace std;

int pid;
int a = 0;
void fk(){
    pid = fork();
    if(pid == 0){
        a=1;
        cout<<"i'm u father"<<endl;;
    }else{
        a=2;
        cout<<"i'm son"<<endl;;
    }
};

int main(){
    fk();
    if(pid == 0){
        cout<<"i'm u father"<<endl;
        cout<<&a;
        cout<<a;
    }else{
        cout<<"i'm son"<<endl;;
        cout<<&a;
        cout<<a;
    }
}