#pragma once
#include <iostream>
#include "task.hpp"
#include <queue>
#include <pthread.h>
#include "../log/log.hpp"
#define THREAD_NUM 6
// 单例模式
class ThreadPool
{
private:
    // 生产者消费者模型
    std::queue<Task> task_queue; // 临界资源
    int thread_count;            // 线程数
    bool stop;                   // 线程池是否停止
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    static ThreadPool *instance;
    bool init_threadPool() // 初始化线程池
    {
        for (int i = 0; i < thread_count; i++)
        {
            pthread_t thread_id;
            if (0 != pthread_create(&thread_id, NULL, thread_routine, this))
            {
                // 创建线程失败
                LOG(FATAL, "init threadpool error!");
                return false;
            }
        }
        LOG(INFO, "http sever init threadpool success");
        return true;
    }
    static void *thread_routine(void *args) // 线程执行函数 static 删除类的this指针,对象通过线程的参数传递
    {
        ThreadPool *pool = (ThreadPool *)args;
        while (!pool->stop)
        {
            Task task;
            pthread_mutex_lock(&pool->mutex);
            while (pool->task_queue.empty()) // while 不能换成if
            {
                pool->thread_wait(); // 线程唤醒后一定占有互斥锁
            }
            task = pool->task_queue.front();
            pool->task_queue.pop();
            pthread_mutex_unlock(&pool->mutex);
            task.ProcessOn();
        }
        return NULL;
    }
    void thread_wait() // 任务队列无任务，线程休眠
    {
        pthread_cond_wait(&cond, &mutex);
    }
    void thread_wakeup() // 任务队列有任务，线程唤醒
    {
        pthread_cond_signal(&cond); // 唤醒一个线程即可
    }
    ThreadPool(int thread_num = THREAD_NUM)
    {
        thread_count = thread_num;
        stop = false;
        pthread_mutex_init(&mutex, nullptr);
        pthread_cond_init(&cond, nullptr);
        init_threadPool();
    }
    ThreadPool(const ThreadPool &) = delete;

public:
    static ThreadPool *GetInstance()
    {
        static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; // 静态锁直接初始化，不需要释放
        if (instance == nullptr)
        {
            pthread_mutex_lock(&lock);
            if (instance == nullptr)
            {
                instance = new ThreadPool();
            }
            pthread_mutex_unlock(&lock);
        }
        return instance;
    }
    ~ThreadPool()
    {
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
    }
    void push_task(const Task &task)
    {
        pthread_mutex_lock(&mutex);
        task_queue.push(task);
        pthread_mutex_unlock(&mutex);
        thread_wakeup();
    }
    bool is_stop()
    {
        return stop;
    }
};

ThreadPool *ThreadPool::instance = nullptr;