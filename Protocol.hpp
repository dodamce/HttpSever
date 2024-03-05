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
#include <unordered_map>

#define SEP ": " // HTTP请求报文分隔符

// 协议读取分析工作(每个线程工作任务)
// 请求报文信息
struct HttpRequest
{
    std::string req_line;               // 请求行
    std::vector<std::string> req_heads; // 请求报头
    std::string blank;                  // 空行
    std::string req_body;               // 请求正文

    // 解析完毕后的数据
    std::string method;                                       // 请求方法
    std::string uri;                                          // 请求路径
    std::string version;                                      // HTTP版本
    std::unordered_map<std::string, std::string> req_headMap; // 请求报头key value形式
    int content_length = 0;                                   // 请求正文长度
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
    void RecvRequestHeads()
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
    // 解析请求报头
    void ParseRequestHeads()
    {
        for (auto &line : request.req_heads)
        {
            // HTTP 以: +空格分隔 请求报头的key value
            std::string key;
            std::string value;
            if (Util::cutString(line, SEP, key, value) == true)
            {
                request.req_headMap.insert({key, value});
            }
        }
    }
    // 是否需要读取正文
    bool HaveReqBody()
    {
        // GET方法默认没有正文，POST方法考虑读取正文
        if (request.method == "POST")
        {
            auto pos = request.req_headMap.find("Content-Length");
            if (pos != request.req_headMap.end())
            {
                request.content_length = atoi(pos->second.c_str()); // 请求正文长度
                return true;
            }
        }
        return false;
    }
    // 读取正文
    void RecvReqBody()
    {
        if (HaveReqBody() == true)
        {
            int length = request.content_length;
            char ch = 0;
            while (length > 0)
            {
                ssize_t size = recv(sock, &ch, 1, 0);
                if (size > 0)
                {
                    request.req_body += ch;
                    length -= 1;
                }
                else
                {
                    break; // 出错，先不考虑
                }
            }
        }
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
    // 读取，解析请求
    void ReadRequest()
    {
        RecvRequestLine();
        RecvRequestHeads();
        ParseRequestLine();
        ParseRequestHeads();
        // 请求正文由Content-length决定
        RecvReqBody();
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
        endpoint->BuildRequest();
        endpoint->SendRequest();
        delete endpoint;
        LOG(INFO, "http request hander end");
        return nullptr;
    }
};