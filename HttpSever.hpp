#pragma once
#include "./TcpSever/TcpSever.hpp"
#include "Protocol.hpp"
#include "./log/log.hpp"
#include <signal.h>
#include <pthread.h>
#include "./ThreadPool/task.hpp"
#include "./ThreadPool/threadpool.hpp"
#define PORT 8080

class HttpSever
{
private:
    int port;
    TcpSever *tcp_sever;
    bool states; // 标记服务器运行状态
    ThreadPool threadpool;
    void InitSever()
    {
        // 信号SIGPIPE需要进行忽略，防止服务器写入管道时失败服务器崩溃
        signal(SIGPIPE, SIG_IGN);
        tcp_sever = TcpSever::GetInstance(port);
    }

public:
    HttpSever(int _port = PORT)
    {
        port = _port;
        tcp_sever = nullptr;
        states = true;
        InitSever();
    }
    ~HttpSever() {}
    void Loop()
    {
        LOG(INFO, "---http sever loop begin---");
        int listen_socket = tcp_sever->GetLinstenSocket();
        while (states != false)
        {
            struct sockaddr_in client; // 客户端信息
            socklen_t len = sizeof(client);
            int sock = accept(listen_socket, (struct sockaddr *)&client, &len);
            if (sock < 0)
            {
                // 套接字监听失败
                continue;
            }
            LOG(INFO, "get a new link");
            int *_sock = new int(sock);
            // 构建任务
            Task task(sock, );
            threadpool.push_task(task);
        }
    }
};