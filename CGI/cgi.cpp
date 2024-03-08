#include <iostream>
#include <stdlib.h>
using namespace std;
int main(int argc, char const *argv[])
{
    // cout被重定向了，不能使用其打印
    // cerr << "DEBUG CGI:" << getenv("METHOD") << endl;
    std::string method = getenv("METHOD");
    cerr << "DEBUG: CGI get method " << method << endl;
    if ("GET" == method)
    {
        string parameter = getenv("Get_Parameter");
        // cerr << "DEBUG CGI GET Parameter:" << parameter << endl;
    }
    else if ("POST" == method)
    {
        cerr << "DEBUG: CGI POST Parameter" << endl;
    }
    else
    {
        // TODO
    }
    return 0;
}
