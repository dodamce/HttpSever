#include <iostream>
#include "./include/mysql.h"
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include "../tool/Util.hpp"
using namespace std;
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
bool insert(std::string &sql)
{
    MYSQL *connect = mysql_init(nullptr);
    mysql_set_character_set(connect, "utf8"); // 设置编码格式，防止乱码
    // mysql connect+ ip + user name + password + database name + port +null+0
    if (mysql_real_connect(connect, "127.0.0.1", "dodamce", "000000", "HttpSever", 3306, nullptr, 0) == nullptr)
    {
        cerr << "DEBUG error connecting!:" << mysql_error(connect) << endl;
        return false;
    }
    cerr << "DEBUG succeed connecting" << endl;
    mysql_close(connect);
    return true;
}
int main(int argc, char const *argv[])
{
    std::string parameter;
    if (GetParameter(parameter))
    {
        // 数据处理
        cerr << "DEBUG: " << parameter << endl;
        // 插入数据库
        // std::string sql = "insert into user (name,passward) values (\'测试\',\'000000\')";
    }
    return 0;
}
