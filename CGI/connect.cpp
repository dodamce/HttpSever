#include <iostream>
#include "./include/mysql.h"
#include <string>
using namespace std;
int main(int argc, char const *argv[])
{
    MYSQL *connect = mysql_init(nullptr);
    mysql_set_character_set(connect, "utf8"); // 设置编码格式，防止乱码
    // mysql connect+ ip + user name + password + database name + port +null+0
    if (mysql_real_connect(connect, "127.0.0.1", "dodamce", "000000", "HttpSever", 3306, nullptr, 0) == nullptr)
    {
        cerr << "DEBUG error connecting!:" << mysql_error(connect) << endl;
        return 1;
    }
    cerr << "DEBUG succeed connecting" << endl;
    std::string sql = "insert into user (name,passward) values (\'测试\',\'000000\')";
    cout << "DEBUG sql:" << sql << endl;
    int ret = mysql_query(connect, sql.c_str());
    if (ret != 0)
    {
        cerr << "DEBUG error querying!:" << mysql_error(connect) << endl;
        return 1;
    }
    mysql_close(connect);  
    return 0;
}
