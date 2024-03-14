#include <iostream>
#include "./include/mysql.h"
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include "../tool/Util.hpp"
#include <sstream>
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
    int ret = mysql_query(connect, sql.c_str());
    if (ret != 0)
    {
        cerr << "DEBUG error query!:" << mysql_error(connect) << endl;
        return false;
    }
    cerr << "DEBUG succeed query" << endl;
    mysql_close(connect);
    return true;
}
// 浏览器传参时，中文字符会被编码为 UTF-8 格式，因此服务器需要对 uri 进行 decode 解码。
//  这个函数会对 uri 进行 decode 解码
std::string decode_uri(const std::string &uri)
{
    std::string result; // 用于保存解码后的 uri
    // 遍历 uri 中的每个字符
    for (size_t i = 0; i < uri.size(); i++)
    {
        if (uri[i] == '%')
        { // 如果当前字符是 '%'
            // 如果当前字符后面还有两个字符，则将这两个字符转换为十进制数
            if (i + 2 < uri.size())
            {
                int value = 0;
                std::istringstream iss(uri.substr(i + 1, 2));
                iss >> std::hex >> value;
                // 将转换后的十进制数转换为对应的字符，并添加到结果中
                result += static_cast<char>(value);
                i += 2;
            }
            else
            { // 如果当前字符后面没有两个字符，则将当前字符添加到结果中
                result += uri[i];
            }
        }
        else
        {                     // 如果当前字符不是 '%'
            result += uri[i]; // 直接将当前字符添加到结果中
        }
    }
    return result; // 返回解码后的 uri
}
int main(int argc, char const *argv[])
{
    std::string parameter;
    if (GetParameter(parameter))
    {
        // 数据处理 拆分参数
        std::string left;
        std::string right;
        Util::cutString(parameter, "&", left, right);
        std::string key;
        std::string value;
        Util::cutString(left, "=", key, value);
        // 对中文名称进行解码
        key = decode_uri(key);
        value = decode_uri(value);
        // cerr << "DEBUG: " << key << ":" << value << endl;
        std::string key2;
        std::string value2;
        Util::cutString(right, "=", key2, value2);
        // cerr << "DEBUG: " << key2 << ":" << value2 << endl;

        // 插入数据库
        std::string sql = "insert into user (name,passward) values (\'";
        sql += value;
        sql += "\',\'";
        sql += value2;
        sql += "\')";
        cerr << "DEBUG: sql: " << sql << endl;
        if (insert(sql))
        {
            // 返回注册成功网页
            cout << "<html>";
            cout << "<head><meta charset=\"UTF-8\"></head>";
            cout << "<body>";
            cout << "<h1>"
                 << "注册成功"
                 << "</h1>";
            cout << "</body>";
            cout << "</html>";
        }
    }
    return 0;
}
