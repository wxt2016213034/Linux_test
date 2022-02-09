#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <thread>
#include <list>
#include "locker.h"
//T是任务类
template <typename T>
class threadpool{
public:
     /*thread_number是线程池中线程的数量，max_requests是请求队列中最多允许的、等待处理的请求的数量*/
    threadpool(int thread_number = 8, int max_requests = 10000);
    ~threadpool();
    bool append(T* request);

private:
    /*工作线程运行的函数，它不断从工作队列中取出任务并执行之*/
    static void* worker(void* arg);
    void run();

private:
    //线程数量
    int m_thread_number;

    //线程池数组；
    std::thread *m_threads;

    //请求队列中最多允许的，等待处理的请求
    int m_max_requests;

    // 请求队列
    std::list< T* > m_workqueue;  

    // 保护请求队列的互斥锁
    locker m_queuelocker;   

    // 是否有任务需要处理
    sem m_queuestat;

    // 是否结束线程          
    bool m_stop;    
};

template<typename T>
threadpool<T>::threadpool(int thread_number, int max_requests):
m_thread_number(thread_number), m_max_requests(max_requests),m_stop(false), m_threads(NULL){
    if(thread_number<0 || max_requests <0){
        throw std::exception();
    }
    this->m_threads = new std::thread[m_thread_number];
    if(!this->m_threads){
        throw std::exception();
    }

    for(int i = 0 ;i < thread_number; ++i){
        this->m_threads[i] = std::thread(worker,this);
        this->m_threads[i].detach();
    }
}

template<typename T>
threadpool<T>::~threadpool(){
    delete[] this->m_threads;
    m_stop = true;
}

template<typename T>
bool threadpool<T>::append(T*request){
    m_queuelocker.lock();
    if(m_workqueue.size() > m_max_requests){
        m_queuelocker.unlock();
        return false;
    }
    m_workqueue.push_back(request);
    m_queuelocker.unlock();
    m_queuestat.post();
    return true;
}

template<typename T>
void *threadpool<T>::worker(void *arg){
    threadpool *pool = static_cast<threadpool*>(arg);
    pool->run();
    return pool;
}

template<typename T>
void threadpool<T>::run(){
    while(!m_stop){
        m_queuestat.wait();
        m_queuelocker.lock();
        if(m_workqueue.empty()){
            m_queuelocker.unlock();
            continue;
        }
        T* request = m_workqueue.front();
        m_workqueue.pop_front();
        m_queuelocker.unlock();
        if(!request){
            continue;
        }
        request->process();
    }
}



#endif
