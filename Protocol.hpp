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
#include <algorithm>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <sys/wait.h>

#define SEP ": " // HTTP请求报文分隔符
// HTTP响应状态码
#define OK 200
#define NOT_FOUND 404
// WEB根目录
#define WEB_ROOT "wwwroot"
// 路径首页
#define HOME_PAGE "index.html"
// HTTP版本
#define HTTP_VERSION "HTTP/1.0"
// 行分割符
#define LINE_END "\r\n"

// 响应状态码描述映射
static std::string StatusMap(int code)
{
    std::string ret;
    switch (code)
    {
    case 200:
        ret = "OK";
        break;
    case 404:
        ret = "Not Found";
        break;
    default:
        break;
    }
    return ret;
}
// 文件类型与HTTP Content-Type映射
static std::string SuffixMap(const std::string &suffix)
{
    static std::unordered_map<std::string, std::string> suffixMap = {
        {".html", "text/html"},
        {".css", "text/css"},
        {".js", "application/x-javascript"},
        {".png", "image/png"},
        {".jpg", "image/jpeg"},
        {".xml", "application/xml"}
        // TODO 其余后续添加
    };
    auto pos = suffixMap.find(suffix);
    if (pos != suffixMap.end())
    {

        return pos->second;
    }
    return "text/html"; // 默认当做html网页处理
}

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

    bool cgi = false;   // 请求是否带参，带参的话服务器调用CGI程序处理
    std::string suffix; // 请求资源后缀
};
// 响应报文信息
struct HttpResponse
{
    std::string res_line;               // 状态行
    std::vector<std::string> res_heads; // 响应报头
    std::string blank = LINE_END;       // 空行
    std::string res_body;               // 响应正文

    int status_code = OK; // 响应状态码
    int fd = -1;          // 响应文件的文件描述符
    size_t size;          // 客户端访问目标文件的大小
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
        // 方法转大写
        auto &method = request.method;
        std::transform(method.begin(), method.end(), method.begin(), ::toupper);
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
    int ProcessNoCGI()
    {
        // 构建响应报文的响应正文,只读方式
        response.fd = open(request.path.c_str(), O_RDONLY);
        if (response.fd > 0)
        {
            // 构建响应行
            auto &res_line = response.res_line;
            res_line = HTTP_VERSION; // HTTP版本
            res_line += " ";
            res_line += std::to_string(response.status_code); // 状态码
            res_line += " ";
            res_line += StatusMap(response.status_code); // 状态码描述
            res_line += LINE_END;
            // 空行初始化为LINE_END
            // 构建响应报头
            std::string content_length = "Content-Length: ";
            content_length += std::to_string(response.size);
            content_length += LINE_END;
            // Content-Type映射表，这里只处理html，css，js，png
            std::string content_type = "Content-Type: ";
            content_type += SuffixMap(request.suffix);
            content_type += LINE_END;
            response.res_heads.push_back(content_length);
            response.res_heads.push_back(content_type);
            return OK;
        }
        return NOT_FOUND;
    }
    int ProcessCGI()
    {
        LOG(INFO, "DEBUG: use cgi method");
        // 父进程数据
        std::string &parameter = request.parameter; // GET方法,环境变量传递信息效率高
        std::string &req_body = request.req_body;   // POST方法,管道传递信息
        std::string &method = request.method;
        std::string &bin = request.path;
        std::string parameter_env;      // 环境变量传参
        std::string method_env;         // 请求方法环境变量，让子进程知道怎么那数据
        std::string content_length_env; // POST方法传递参数长度
        int cgi_status = OK;            // 函数运行状态
        // 使用匿名管道实现子进程和CGI程序通信,站在父进程角度进行input output
        // 父进程通过input来读取CGI程序数据，output来向CGI程序提供数据。
        // 子进程通过向input写入数据，output拿取数据
        int input[2] = {0};
        int output[2] = {0};
        if (pipe(input) < 0)
        {
            LOG(ERROR, "pip input error!");
            cgi_status = NOT_FOUND;
            return cgi_status;
        }
        if (pipe(output) < 0)
        {
            LOG(ERROR, "pip output error!");
            cgi_status = NOT_FOUND;
            return cgi_status;
        }
        // 线程进入CGI内部，创建子进程执行CGI程序
        pid_t pid = fork();
        if (pid == 0) // 子进程
        {
            close(input[0]);
            close(output[1]);
            // 方法也给子进程
            method_env = "METHOD=" + method;
            putenv((char *)method_env.c_str());
            // LOG(INFO, "DEBUG: put method env " + method_env);
            if (method == "GET")
            {
                parameter_env = "Get_Parameter=" + parameter;
                putenv((char *)parameter_env.c_str());
            }
            else if (method == "POST")
            {
                content_length_env = "Content_Length=" + std::to_string(request.content_length); // 将参数长度写入环境变量
                putenv((char *)content_length_env.c_str());
            }
            else
            {
                // TODO 其余方法先不处理
            }
            // 重定向，1 input[1]写 ; 0 output[0]读取
            dup2(input[1], 1);
            dup2(output[0], 0);
            //  执行目标程序,目标子进程通过使用重定向原则，让替换后的进程读取管道数据向标准输入读取，写入数据向标准输出写入即可。在进程替换前进行重定向
            execl(bin.c_str(), bin.c_str(), nullptr);
            // 未替换成功
            LOG(ERROR, "execl error!");
            exit(1);
        }
        else if (pid < 0) // 创建子进程失败
        {
            LOG(ERROR, "thread fork error!");
            return NOT_FOUND;
        }
        else // 父进程
        {
            close(input[1]);
            close(output[0]);
            if (method == "POST") // 数据在body上,写入管道
            {
                const char *start = req_body.c_str();
                size_t total = 0; // 一共写入了多少字节
                size_t size = 0;  // 本次写入内容大小
                while (total < req_body.size() && (size = write(output[1], start + total, req_body.size() - total)) > 0)
                {
                    total += size;
                }
            }
            // 读cgi发送回的数据
            char ch = 0;
            while (read(input[0], &ch, 1) > 0)
            {
                // 构建响应正文发送给客户端
                response.res_body += ch;
            }
            int status = 0;                       // 子进程运行状态
            pid_t ret = waitpid(pid, &status, 0); // 阻塞进程等待，返回码不关注设为nullptr
            if (ret == pid)
            {
                // 等待成功，子进程可能出现错误，判断子进程运行情况
                if (WIFEXITED(status))
                {
                    // 正常退出，且退出码为0，说明正确运行
                    if (WEXITSTATUS(status) == 0)
                    {
                        cgi_status = OK;
                    }
                    else
                    {
                        cgi_status = NOT_FOUND;
                    }
                }
                else
                {
                    cgi_status = NOT_FOUND;
                }
            }
            close(input[0]);
            close(output[1]);
        }
        return cgi_status;
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
    void BuildResponse()
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
                    request.cgi = true;
                }
                else
                {
                    request.path = request.uri;
                }
            }
            else if (method == "POST")
            {
                // POST方法,带参数，服务器请求CGI处理数据
                request.cgi = true;
                request.path = request.uri;
            }
            else
            {
                // 其他方法，先不处理
                // TODO
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
                    // 获取新文件属性，更新属性信息
                    stat(request.path.c_str(), &file_state);
                }
                // 如果请求的是可执行程序，特殊处理
                if ((file_state.st_mode & S_IXUSR) || (file_state.st_mode & S_IXGRP) || (file_state.st_mode & S_IXOTH))
                {
                    // 请求可执行程序,服务器调用可程序
                    request.cgi = true;
                }
                response.size = file_state.st_size;
                // 资源存在确认请求后缀
                size_t found = request.path.rfind(".");
                if (found == std::string::npos)
                {
                    request.suffix = ".html"; // 默认html文件
                }
                else
                {
                    request.suffix = request.path.substr(found);
                }
            }
            else
            {
                // 资源不存在，404错误
                response.status_code = NOT_FOUND;
                LOG(WARNING, request.path + " not found");
                goto END;
            }
            LOG(INFO, "DEBUG: " + request.path);
        }
        else
        {
            // 非法请求，构建错误响应报文
            LOG(WARNING, "method is not right");
            response.status_code = NOT_FOUND;
            goto END;
        }
        if (request.cgi == true)
        {
            // 以CGI的方式处理请求
            response.status_code = ProcessCGI();
        }
        else
        {
            // 非CGI方式处理数据，GET方法不带参数，构建HTTP响应，文本网页返回
            response.status_code = ProcessNoCGI();
        }
    END:
        if (response.status_code != OK)
        {
            // 错误
            // TODO
        }
    }
    // 发送响应
    void SendResponse()
    {
        send(sock, response.res_line.c_str(), response.res_line.size(), 0); // 阻塞发送
        for (auto &item : response.res_heads)
        {
            send(sock, item.c_str(), item.size(), 0); // 记得带LINE_END
        }
        send(sock, response.blank.c_str(), response.blank.size(), 0);
        // 发送正文
        sendfile(sock, response.fd, nullptr, response.size);
        // 关闭文件
        close(response.fd);
    }
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
        endpoint->BuildResponse();
        endpoint->SendResponse();
        delete endpoint;
        LOG(INFO, "http request hander end");
        return nullptr;
    }
};