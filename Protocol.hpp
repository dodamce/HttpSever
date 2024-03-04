#pragma once
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "./tool/Util.hpp"
#include <string>
#include <vector>
#include "./log/log.hpp"
#include <sstream>

// 协议读取分析工作(每个线程工作任务)
// 请求报文信息
struct HttpRequest
{
    std::string req_line;               // 请求行
    std::vector<std::string> req_heads; // 请求报头
    std::string blank;                  // 空行
    std::string req_body;               // 请求正文

    // 解析完毕后的数据
    std::string method;  // 请求方法
    std::string uri;     // 请求路径
    std::string version; // HTTP版本
};
// 响应报文信息
struct HttpResponse
{
    std::string res_line;               // 状态行
    std::vector<std::string> res_heads; // 响应报头
    std::string blank;                  // 空行
    std::string res_body;               // 响应正文
};
// 对端业务逻辑类,读取请求，分析请求，构建响应。基本IO通信
class EndPoint
{
private:
    int sock;
    HttpRequest request;
    HttpResponse response;

    // 读取请求行
    void RecvRequestLine()
    {
        Util::readLine(sock, request.req_line);
        request.req_line.resize(request.req_line.size() - 1); // 删除\n
        // LOG(INFO, request.req_line);
    }
    // 读取请求报头
    void RecvRequestHander()
    {
        std::string line;
        while (true)
        {
            line.clear();
            Util::readLine(sock, line);
            if (line == "\n")
            {
                request.blank = line;
                break; // line==\n时读取到空行
            }
            line.resize(line.size() - 1); // 删除最后一个字符\n
            request.req_heads.push_back(line);
            // LOG(INFO, line);
        }
    }
    // 解析请求头
    void ParseRequestLine()
    {
        std::string &line = request.req_line;
        std::stringstream str(line);
        str >> request.method >> request.uri >> request.version;
        // LOG(INFO, request.method + ":" + request.uri + ":" + request.version);
    }

public:
    EndPoint(int _sock)
    {
        sock = _sock;
    }
    ~EndPoint()
    {
        close(sock);
    }
    // 读取请求
    void ReadRequest()
    {
        RecvRequestLine();
        RecvRequestHander();
    }
    // 解析请求
    void ParseRequest()
    {
        ParseRequestLine();
    }
    // 构建响应
    void BuildRequest() {}
    // 发送响应
    void SendRequest() {}
};
// 线程工作入口
class Entrance
{
public:
    // 处理HTTP请求
    static void *HanderReq(void *_sock)
    {
        LOG(INFO, "http request hander begin");
        int sock = *(int *)_sock;
        delete (int *)_sock;
        EndPoint *endpoint = new EndPoint(sock);
        endpoint->ReadRequest();
        endpoint->ParseRequest();
        endpoint->BuildRequest();
        endpoint->SendRequest();
        delete endpoint;
        LOG(INFO, "http request hander end");
        return nullptr;
    }
};