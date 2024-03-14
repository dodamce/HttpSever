#include <iostream>
#include "./include/mysql.h"
using namespace std;
int main(int argc, char const *argv[])
{ 
    cout << mysql_get_client_info() << endl;
    return 0;
}
