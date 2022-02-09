#ifndef LOCKER_H
#define LOCKER_H

#include <mutex>
#include <condition_variable>
#include <exception>
#include <semaphore.h>

// 线程同步机制封装类

//互斥锁类
class locker{
    public:

        locker();
        ~locker();

        bool lock();
        bool unlock();

        std::mutex& get();

    private:

        std::mutex m_mutex;
};


//条件变量类
class condVar{
    public:
        condVar();
        ~condVar();

        bool wait(std::mutex &m_mutex);
        bool timewait(std::mutex &m_mutex, int time);

        bool notify_one();
        bool notify_all();

    private:
        std::condition_variable my_condVar;
};

// 信号量类
class sem {
public:
    sem() {
        if( sem_init( &m_sem, 0, 0 ) != 0 ) {
            throw std::exception();
        }
    }
    sem(int num) {
        if( sem_init( &m_sem, 0, num ) != 0 ) {
            throw std::exception();
        }
    }
    ~sem() {
        sem_destroy( &m_sem );
    }
    // 等待信号量
    bool wait() {
        return sem_wait( &m_sem ) == 0;
    }
    // 增加信号量
    bool post() {
        return sem_post( &m_sem ) == 0;
    }
private:
    sem_t m_sem;
};

#endif
