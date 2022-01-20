#include <signal.h>
#include <stdio.h>
#include <unistd.h>
void interrupt(int a) {
printf("Interrupt called\n"); 
sleep(5);
printf("Interrupt Func Ended.\n");
};
void catchquit(int b) {
printf("Quit called\n"); 
sleep(3);
printf("Quit ended.\n");
} ;
int main() {
signal(SIGINT,interrupt); signal(SIGQUIT,catchquit); printf("Interrupt set for SIGINT\n"); sleep(10);
printf("Program NORMAL ended.\n");
return 0; 
}