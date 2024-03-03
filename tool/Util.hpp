// 项目使用到的基本工具方法
#pragma once
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
class Util
{
public:
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
};