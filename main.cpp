#include "HttpSever.hpp"
#include <iostream>
#include <string>
#include <memory>

void Usage(std::string proc)
{
    std::cout << "Usage:\n\t" << proc << " + port" << std::endl;
}

int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        Usage(argv[0]);
        exit(4);
    }
    int port = atoi(argv[1]);
    std::shared_ptr<HttpSever> sever(new HttpSever(port));
    sever->Loop();
    return 0;
}
