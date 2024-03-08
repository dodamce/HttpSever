#include <iostream>
#include <stdlib.h>
using namespace std;
int main(int argc, char const *argv[])
{
    //cout被重定向了，不能使用其打印
    cerr << "DEBUG CGI:" << getenv("METHOD") << endl;
    return 0;
}
