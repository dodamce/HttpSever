// 项目使用到的基本工具方法
#pragma once
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unordered_map>
class Util
{
public:
    // 时间戳转化为时间信息
    static std::string convertTimeStamp2TimeStr(time_t timeStamp)
    {
        struct tm *timeinfo = nullptr;
        char buffer[80];
        timeinfo = localtime(&timeStamp);
        strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", timeinfo);
        // printf("%s\n", buffer);
        return std::string(buffer);
    }
    // 套接字读取流中的一行
    static int readLine(int sock, std::string &out)
    {
        char ch = 0;
        // 统一转化为\n结尾
        while (ch != '\n')
        {
            ssize_t size = recv(sock, &ch, 1, 0);
            if (size > 0)
            {
                if (ch == '\r')
                {
                    // 转化为\n,嗅探下一个字符 MSG_PEEK
                    recv(sock, &ch, 1, MSG_PEEK);
                    if (ch == '\n')
                    {
                        recv(sock, &ch, 1, 0);
                    }
                    else
                    {
                        ch = '\n';
                    }
                }
                out += ch;
            }
            else if (size == 0)
            {
                // 对端关闭
                return 0;
            }
            else
            {
                return -1;
            }
        }
        return out.length();
    }
    // 切分字符串src,结果放到left，rigth上
    static bool cutString(std::string &src, const std::string sep, std::string &left, std::string &right)
    {
        size_t pos = src.find(sep);
        if (pos != std::string::npos)
        {
            left = src.substr(0, pos);
            right = src.substr(pos + sep.length());
            return true;
        }
        return false;
    }
};