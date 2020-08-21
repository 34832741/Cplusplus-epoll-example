#include <iostream>
#include "myepoll.h"

using namespace std;

int main() {
    cout << "Hello, World!" << std::endl;

    myepoll myepoll;
    int ret = 0;
    ret = myepoll.start();
    if (ret == -1)
    {
        cout << myepoll.get_socketfd();
        exit(-1);
    }

    ret = myepoll.do_epoll();
    if (ret == -1)
    {
        cout << myepoll.get_socketfd();
        exit(-1);
    }

    return 0;
}
