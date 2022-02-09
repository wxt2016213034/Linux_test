#include "locker.h"
locker::locker(){

}
locker::~locker(){
    
}
bool locker::lock(){
    this->m_mutex.lock();
    return true;
}

bool locker::unlock(){
    this->m_mutex.unlock();
    return true;
}

std::mutex* locker::get(){
    return &m_mutex;
}

bool condVar::wait(std::mutex &m_mutex){
    std::unique_lock<std::mutex> temp(m_mutex);
    this->my_condVar.wait(temp);
    return true;
}


bool condVar::timewait(std::mutex &m_mutex,int time){
    std::unique_lock<std::mutex> temp(m_mutex);
    this->my_condVar.wait_for(temp,std::chrono::seconds(time));
    return true;

}


bool condVar::notify_one(){
    this->my_condVar.notify_one();
    return true;

}


bool condVar::notify_all(){
    this->my_condVar.notify_all();
    return true;

}