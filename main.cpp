#include "TcpSever.hpp"
#include <iostream>
#include <string>

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
    TcpSever *sever = TcpSever::GetInstance(atoi(argv[1]));
    std::cout << "Hello Sever" << std::endl;
    return 0;
}
