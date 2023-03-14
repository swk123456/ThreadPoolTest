#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#include<string>
#include<string.h>
#include<unistd.h>
#include<iostream>
#include"TaskQueue.h"

class ThreadPool
{
public:
    ThreadPool(int min, int max, int add = 1);
    ~ThreadPool();
    
    void addTask(Task task);

    int getBusyNum();
    int getAliveNum();

    void waitForEnd();

private:
    TaskQueue *m_taskQ;//任务队列
    pthread_t m_mangerID;//管理者线程ID
    pthread_t *m_threadID;//工作线程ID数组
    int m_minNum;//最小线程数
    int m_maxNum;//最大线程数
    int m_addNum;//一次添加的线程数
    int m_busyNum;//忙的线程数
    int m_liveNum;//存活线程数
    int m_exitNum;//要销毁的线程数
    pthread_mutex_t m_mutexPool;//线程池的锁
    pthread_cond_t m_notEmpty;//条件变量，任务队列是否为空
    bool m_shutdown = false;//是否销毁线程池

    static void* worker(void* arg);//工作线程任务函数
    static void* manger(void* arg);//管理者线程任务函数

    void threadExit();//单个线程退出
};

#endif