#include"ThreadPool.h"

ThreadPool::ThreadPool(int min, int max, int add)
     : m_minNum(min), m_maxNum(max), m_addNum(add),
       m_busyNum(0), m_liveNum(min)
{
    m_taskQ = new TaskQueue;
    if(m_taskQ == nullptr) {
        std::cout << "malloc taskQ fail" << std::endl;
    }
    m_threadID = new pthread_t[max];
    if(m_threadID == nullptr) {
        std::cout << "malloc thread_t fail" << std::endl;
    }
    memset(m_threadID, 0, sizeof(pthread_t) * max);
    if(pthread_mutex_init(&m_mutexPool, NULL) != 0
     || pthread_cond_init(&m_notEmpty, NULL) != 0) {
        std::cout << "init mutex or condition fail" << std::endl;
    }
    m_shutdown = false;
    for(int i = 0; i < min; ++i) {
        pthread_create(&m_threadID[i], NULL, worker, this);
        std::cout << "create thread id:" << std::to_string(m_threadID[i]) << std::endl;
    }
    pthread_create(&m_mangerID, NULL, manger, this);
}

ThreadPool::~ThreadPool()
{
    m_shutdown = true;
    pthread_join(m_mangerID, NULL);
    for(int i = 0; i < m_liveNum; ++i) {
        pthread_cond_signal(&m_notEmpty);
    }
    if(m_threadID) {
        delete []m_threadID;
    }
    pthread_mutex_destroy(&m_mutexPool);
    pthread_cond_destroy(&m_notEmpty);
}
    
void ThreadPool::addTask(Task task)
{
    if(m_shutdown) {
        return;
    }
    m_taskQ->addTask(task);
    pthread_cond_init(&m_notEmpty, NULL);
}

int ThreadPool::getBusyNum()
{
    pthread_mutex_lock(&m_mutexPool);
    int busyNum = m_busyNum;
    pthread_mutex_unlock(&m_mutexPool);
    return busyNum;
}

int ThreadPool::getAliveNum()
{
    pthread_mutex_lock(&m_mutexPool);
    int aliveNum = m_liveNum;
    pthread_mutex_unlock(&m_mutexPool);
    return aliveNum;
}

void* ThreadPool::worker(void* arg)
{
    ThreadPool *pool = static_cast<ThreadPool*>(arg);
    while(true) {
        pthread_mutex_lock(&pool->m_mutexPool);//访问任务队列加锁

        //判断队列是否为空，为空则工作线程阻塞
        while(pool->m_taskQ->getTaskNum() == 0 && !pool->m_shutdown) {
            std::cout << "thread " << std::to_string(pthread_self()) << " waiting" << std::endl;
            pthread_cond_wait(&pool->m_notEmpty, &pool->m_mutexPool);//阻塞线程

            //解除阻塞后，判断是否销毁线程
            if(pool->m_exitNum > 0) {
                pool->m_exitNum--;
                if(pool->m_liveNum > pool->m_minNum) {
                    pool->m_exitNum--;
                    pthread_mutex_unlock(&pool->m_mutexPool);
                    pool->threadExit();
                }
            }
        }

        if(pool->m_shutdown) {//判断线程池是否关闭
            pthread_mutex_unlock(&pool->m_mutexPool);
            pool->threadExit();
        }

        Task task = pool->m_taskQ->takeTask();//取出一个任务
        pool->m_busyNum++;
        pthread_mutex_unlock(&pool->m_mutexPool);//线程池解锁

        //执行任务
        std::cout << "thread " << std::to_string(pthread_self()) << " start working" << std:: endl;
        task.function(task.arg);//任务函数执行时阻塞
        delete task.arg;
        task.arg = nullptr;

        //任务处理结束
        std::cout << "thread " << std::to_string(pthread_self()) << " end working" << std::endl;
        pthread_mutex_lock(&pool->m_mutexPool);
        pool->m_busyNum--;
        pthread_mutex_unlock(&pool->m_mutexPool);
    }

    return nullptr;
}

//管理者线程，主要管理线程池中的线程数，增加或销毁线程
void* ThreadPool::manger(void* arg)
{
    ThreadPool *pool = static_cast<ThreadPool*>(arg);
    while(!pool->m_shutdown) {
        sleep(5);//每隔一段时间检测一次
        //取出线程池中的任务数和线程数
        //取出当前工作的线程数
        pthread_mutex_lock(&pool->m_mutexPool);
        int queueSize = pool->m_taskQ->getTaskNum();
        int liveNum = pool->m_liveNum;
        int busyNum = pool->m_busyNum;
        pthread_mutex_unlock(&pool->m_mutexPool);

        //如果当前任务数>存活线程数&&存活线程数<最大线程数，则添加线程
        if(queueSize > liveNum && liveNum < pool->m_maxNum) {
            pthread_mutex_lock(&pool->m_mutexPool);
            int num = 0;
            for(int i = 0; i < pool->m_maxNum && num < pool->m_addNum
              && pool->m_liveNum < pool->m_maxNum; ++i) {
                if(pool->m_threadID[i] == 0) {
                    pthread_create(&pool->m_threadID[i], NULL, worker, pool);
                    std::cout << "create thread id:" << std::to_string(pool->m_threadID[i]) << std::endl;
                    ++num;
                    ++pool->m_liveNum;
                }
            }
            pthread_mutex_unlock(&pool->m_mutexPool);
        }
        //如果当前忙线程数*2<存活线程数&&存活线程数>最小线程数，销毁多余线程
        if(busyNum * 2 < liveNum && liveNum > pool->m_minNum) {
            pthread_mutex_lock(&pool->m_mutexPool);
            pool->m_exitNum = pool->m_addNum;
            pthread_mutex_unlock(&pool->m_mutexPool);
            //让工作线程自杀
            for(int i = 0; i < pool->m_addNum; ++i) {
                //唤醒条件变量的阻塞，跳转到worker函数看是否要销毁线程
                pthread_cond_signal(&pool->m_notEmpty);
            }
        }
    }
    return nullptr;
}

void ThreadPool::threadExit()
{
    pthread_t tid = pthread_self();
    for(int i = 0; i < m_maxNum; ++i) {
        if(m_threadID[i] == tid) {
            std::cout << "threadExit function thread " << std::to_string(pthread_self()) << " exit" << std::endl;
            m_threadID[i] = 0;
            break;
        }
    }
    pthread_exit(NULL);
}

void ThreadPool::waitForEnd()
{
    for(int i = 0; i < m_maxNum; ++i) {
        if(m_threadID[i] != 0) {
            pthread_join(m_threadID[i], NULL);
        }
    }
}