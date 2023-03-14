#include"./ThreadPool/ThreadPool.h"
#include<iostream>
#include<unistd.h>

void taskFunc(void* arg)
{
    int num = *(int*)arg;
    std::cout << "thread " << pthread_self() << " is working, num = " << num << std::endl;
    sleep(1);
}

int main()
{
    ThreadPool pool(4, 10, 4);
    for(int i = 0; i < 100; ++i) {
        int *num = new int(i);
        pool.addTask(Task(taskFunc, num));
    }
    pool.waitForEnd();
    return 0;
}