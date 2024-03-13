#pragma once

#include <iostream>
#include "../Protocol.hpp"

// 设计线程任务队列
class Task
{
private:
    int sock;
    CallBack callback; // 设置回调
public:
    Task() {}
    Task(int _sock)
    {
        sock = _sock;
    }
    void ProcessOn()
    {
        callback(sock);
    }
};