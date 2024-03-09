#include <iostream>
#include <stdlib.h>
#include <unistd.h>
using namespace std;
int main(int argc, char const *argv[])
{
    // cout被重定向了，不能使用其打印
    // cerr << "DEBUG CGI:" << getenv("METHOD") << endl;
    std::string method = getenv("METHOD");
    cerr << "DEBUG: CGI get method " << method << endl;
    string parameter;
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
        while (content_length > 0)
        {
            read(0, &ch, 1);
            parameter += ch;
            content_length -= 1;
        }
        cerr << "DEBUG: POST CGI end msg= " << parameter << endl;
    }
    else
    {
        // TODO
    }
    return 0;
}
