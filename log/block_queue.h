#ifndef BLOCK_QUEUE_H
#define BLOCK_QUEUE_H

#include <iostream>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <../lock/locker.h>

using namespace std;
//用多少线程把缓存去写进文件？
template <class T>
class block_queue
{
public:
    block_queue(int max_size = 1000){
        if(max_size<=0){
            exit(-1);
        }
        m_max_size = max_size;
        m_size = 0;
        m_array = new T[max_size];
        m_front = -1;
        m_back = -1;
    }

    ~block_queue(){
        m_mutex.lock();
        if(m_array!=nullptr){
            delete[] m_array;
            m_array=nullptr;
        }
        m_mutex.unlock();
    }

    bool full(){
        m_mutex();
        if(m_size>=m_max_size){
            m_mutex.unlock();
            return true;
        }
        m_mutex.unlock();
        return false;
    }

    bool empty(){
        m_mutex.lock();
        if(m_size==0){
            m_mutex.unlock();
            return true;
        }
        m_mutex.unlock();
        return false;
    }

    bool front(T& value){
        m_mutex.lock();
        if(empty()){
            m_mutex.unlock()
            return false;
        }
        value=m_array[m_front];
        m_mutex.unlock();
        return true;
    }

    bool back(T& value){
        m_mutex.lock();
        if(empty()){
            m_mutex.unlock();
            return false;
        }
        value=m_array[m_back];
        m_mutex.unlock();
        return true;
    }

    int size(){
        m_mutex.lock();
        int tmp=m_size;
        m_mutex.unlock();
        return tmp;
    }

    int max_size(){
        m_mutex.lock();
        int tmp=m_max_size;
        m_mutex.unlock();
        return tmp;
    }
//这里后续需要处理 因为可能会push失败 导致丢失日志
    bool push(T& value){
        m_mutex.lock();
        if(full()){
            m_cond.broadcast();
            m_mutex.unlock();
            return false;
        }
        if(m_front==m_back&&empty()){
            m_array[(++back)%m_max_size]=value;
            m_front=(m_front+1)%m_max_size;
            m_size++;
            m_cond.broadcast();
            m_mutex.unlock();
            return true;
        }
        m_array[++m_back]==value;
        m_size++;
        m_cond.broadcast();
        m_mutex.unlock();
        return true;
    }

    bool pop(T& value){
        m_mutex.lock();
        while(empty()){
            if(!m_cond.wait(&m_mutex.get())){
                m_mutex.unlock();
                return false;
            }
        }
         if(m_size==1){
                value=m_array[m_front];
                m_size--;
                m_mutex.unlock();
                return true;
            }
        value=m_array[m_front];
        m_front=(m_front+1)%m_max_size;
        m_size--;
        m_mutex.unlock();
        return true;
    }

private:
    locker m_mutex;
    cond m_cond;

    T *m_array;
    int m_size;
    int m_max_size;
    int m_front;
    int m_back;
};


#endif
