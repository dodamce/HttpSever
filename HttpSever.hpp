#pragma once
#include "./TcpSever/TcpSever.hpp"
#include "Protocol.hpp"
#include "./log/log.hpp"
#define PORT 8080

class HttpSever
{
private:
    int port;
    TcpSever *tcp_sever;
    bool states; // 标记服务器运行状态
    void InitSever()
    {
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
            pthread_t tid = 0;
            pthread_create(&tid, nullptr, Entrance::HanderReq, _sock);
            pthread_detach(tid); // 线程分离
        }
    }
};