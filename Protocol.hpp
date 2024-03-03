#pragma once
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "./tool/Util.hpp"

// 协议读取分析工作(每个线程工作任务)

// 线程工作入口
class Entrance
{
public:
    // 处理HTTP请求
    static void *HanderReq(void *_sock)
    {
        int sock = *(int *)_sock;
        delete (int *)_sock;
        std::cout << "Get a new link sock=" << sock << std::endl;
        std::string line;
        Util::readLine(sock, line);
        std::cout << line << std::endl;
        close(sock);
        return nullptr;
    }
};