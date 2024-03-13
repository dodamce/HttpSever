#pragma once

#include <iostream>

// 设计线程任务队列
class Task
{
private:
    int sock;
    CallBack callback; // 设置回调
public:
    Task() {}
    Task(int _sock, CallBack _callback)
    {
        sock = _sock;
        callback = _callback;
    }
    void ProcessOn()
    {
        callback(sock);
    }
};