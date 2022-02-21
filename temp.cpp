
#include <semaphore.h>
#include <iostream>
#include <thread>
#include <queue>
int a = 0;
sem_t m_sem;

void func(){
    sem_wait( &m_sem );
    for(int i = 0; i < 10000; ++i)++a;
    sem_post(&m_sem);
}

int main(){
    sem_init( &m_sem, 0, 1 );
    std::thread thread1(func);
    std::thread thread2(func);
    thread1.join();
    thread2.join();
    std::cout<<a;
}