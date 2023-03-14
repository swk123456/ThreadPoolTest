//任务队列
#ifndef _TASK_QUEUE_H_
#define _TASK_QUEUE_H_

#include<queue>
#include<pthread.h>

//using定义别名
//定义callback是指向参数是void*，返回值是void的函数指针
using callback = void (*)(void*);

//任务结构体，接受要处理的任务
struct Task
{
    callback function;
    void *arg;

    Task():function(nullptr), arg(nullptr){}
    Task(callback f, void* arg1)
         :function(f), arg(arg1) {}
};

//任务队列
class TaskQueue
{
private:
    pthread_mutex_t m_mutex;
    std::queue<Task> m_taskQ;

public:
    TaskQueue();
    ~TaskQueue();

    //添加任务
    void addTask(Task task);
    void addTask(callback func, void* arg);
    //取出任务
    Task takeTask();

    //获取当前队列中任务数量
    int getTaskNum();
};

#endif