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
#include <sys/stat.h>

#define SEP ": " // HTTP请求报文分隔符
// HTTP响应状态码
#define OK 200
#define NOT_FOUND 404
// WEB根目录
#define WEB_ROOT "wwwroot"
// 路径首页
#define HOME_PAGE "index.html"

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
    std::string uri;                                          // 请求路径,可能带参数
    std::string version;                                      // HTTP版本
    std::unordered_map<std::string, std::string> req_headMap; // 请求报头key value形式
    int content_length = 0;                                   // 请求正文长度
    std::string path;                                         // 请求资源路径
    std::string parameter;                                    // GET请求携带的参数
};
// 响应报文信息
struct HttpResponse
{
    std::string res_line;               // 状态行
    std::vector<std::string> res_heads; // 响应报头
    std::string blank;                  // 空行
    std::string res_body;               // 响应正文

    int status_code = 0; // 响应状态码
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
    void BuildRequest()
    {
        // 只处理POST/GET请求
        std::string &method = request.method;
        if (method == "POST" || method == "GET")
        {
            // 判断GET方法上URL是否带参数
            if (method == "GET")
            {
                if (request.uri.find("?") != std::string::npos)
                {
                    // 带参 切分字符串，左边为路径，右边为参数
                    Util::cutString(request.uri, "?", request.path, request.parameter);
                    // LOG(INFO, request.path + "------" + request.parameter);
                }
                else
                {
                    request.path = request.uri;
                }
            }
            // 重新修改路径，让其指向WEB服务器根路径wwwroot/ 默认首页index.hmtl
            request.path.insert(0, WEB_ROOT);
            if (request.path[request.path.size() - 1] == '/')
            {
                // 路径以/结尾，默认访问这个路径的首页
                request.path += HOME_PAGE;
            }
            // LOG(INFO, request.path);
            // 判断文件路径是否存在
            struct stat file_state;
            if (stat(request.path.c_str(), &file_state) == 0)
            {
                // 文件存在,判断是否是路径
                if (S_ISDIR(file_state.st_mode))
                {
                    // 请求路径存在，但是是目录，默认到这个路径的首页上
                    request.path += "/";
                    request.path += HOME_PAGE;
                }
                // 如果请求的是可执行程序，特殊处理
                if ((file_state.st_mode & S_IXUSR) || (file_state.st_mode & S_IXGRP) || (file_state.st_mode & S_IXOTH))
                {
                    // 请求可执行程序
                }
            }
            else
            {
                // 资源不存在，404错误
                response.status_code = NOT_FOUND;
                LOG(WARNING, request.path + "not found");
                goto END;
            }
        }
        else
        {
            // 非法请求，构建错误响应报文
            LOG(WARNING, "method is not right");
            response.status_code = NOT_FOUND;
            goto END;
        }
    END:
    }
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