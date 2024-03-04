#pragma once
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "../log/log.hpp"
#include <unistd.h>
#define BACKLOG 5
// 封装套接字,单例模式
class TcpSever
{
private:
    int port;
    int listen_socket;
    TcpSever(int _port)
    {
        port = _port;
        listen_socket = -1;
    }
    TcpSever(const TcpSever &sever) {}

    // 创建套接字
    void Socket()
    {
        // IPV4+TCP
        listen_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (listen_socket < 0)
        {
            LOG(FATAL, "listen socket create error!");
        }
        LOG(INFO, "listen socket create success");
        // 设置套接字复用
        int opt = 1;
        setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    }
    // 绑定监听套接字
    void Bind()
    {
        struct sockaddr_in sever;
        memset(&sever, 0, sizeof(sever));
        sever.sin_family = AF_INET;
        sever.sin_port = htons(port);       // 转化成网络字节序列
        sever.sin_addr.s_addr = INADDR_ANY; // 云服务器不能直接绑定公网ip
        if (bind(listen_socket, (struct sockaddr *)&sever, sizeof(sever)) < 0)
        {
            LOG(FATAL, "bind error!");
        }
        LOG(INFO, "bind listen socket success");
    }
    // 设置套接字为监听状态
    void Listen()
    {

        if (listen(listen_socket, BACKLOG) < 0)
        {
            LOG(FATAL, "listen error!");
        }
        LOG(INFO, "socket listen success");
    }
    void InitSever()
    {
        Socket();
        Bind();
        Listen();
        LOG(INFO, "tcp sever init success");
    }
    ~TcpSever()
    {
        if (listen_socket >= 0)
        {
            close(listen_socket);
        }
    }

public:
    static TcpSever *sever;
    static TcpSever *GetInstance(int port)
    {
        static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
        if (sever == nullptr)
        {
            pthread_mutex_lock(&lock);
            if (sever == nullptr)
            {
                sever = new TcpSever(port);
                sever->InitSever();
            }
            pthread_mutex_unlock(&lock);
        }
        return sever;
    }
    int GetLinstenSocket()
    {
        return listen_socket;
    }
};

TcpSever *TcpSever::sever = nullptr;