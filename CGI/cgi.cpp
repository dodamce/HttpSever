#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include "../tool/Util.hpp"
using namespace std;
// 获取父进程转递的信息，信息写入parameter上
bool GetParameter(string &parameter)
{
    // cout被重定向了，不能使用其打印
    // cerr << "DEBUG CGI:" << getenv("METHOD") << endl;
    std::string method = getenv("METHOD");
    cerr << "DEBUG: CGI get method " << method << endl;
    bool flag = true;
    if ("GET" == method)
    {
        parameter = getenv("Get_Parameter");
        // cerr << "DEBUG CGI GET Parameter:" << parameter << endl;
    }
    else if ("POST" == method)
    {
        // cerr << "DEBUG: CGI POST Parameter" << endl;
        int content_length = atoi(getenv("Content_Length"));
        // cerr << "DEBUG: Countent-Length: " << content_length << endl;
        char ch = 0;
        while (content_length > 0) // 从管道文件中写入
        {
            read(0, &ch, 1);
            parameter += ch;
            content_length -= 1;
        }
        cerr << "DEBUG: POST CGI end msg= " << parameter << endl;
    }
    else
    {
        // TODO 其余方法不处理,默认处理为错误
        flag = false;
    }
    return flag;
}
int main(int argc, char const *argv[])
{
    string parameter;
    if (GetParameter(parameter) == true)
    {
        // 拆分参数
        std::string left;
        std::string right;
        Util::cutString(parameter, "&", left, right);
        std::string key;
        std::string value;
        Util::cutString(left, "=", key, value);
        // cerr << "DEBUG: " << key << ":" << value << endl;
        std::string key2;
        std::string value2;
        Util::cutString(right, "=", key2, value2);
        // cerr << "DEBUG: " << key2 << ":" << value2 << endl;

        // 重定向标准输出，直接向标准输出打印字符，父进程可以通过read读取
        cout << "DEBUG CGI send: " << key << ":" << value << endl;
        cout << "DEBUG CGI send: " << key2 << ":" << value2 << endl;
    }
    return 0;
}
