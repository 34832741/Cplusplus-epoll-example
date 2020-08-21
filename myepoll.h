//
// Created by on 2020/8/9.
//

#ifndef EPOLLTEST_MYEPOLL_H
#define EPOLLTEST_MYEPOLL_H

#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <errno.h>
#include <iostream>
#include <stdio.h>
#include <time.h>
#include <stdio.h>
#include <cstdlib>
using namespace std;

const int MAXBUFFSIXE = 1024;
const int MAXEVENTS = 500;
const int FDSIZE = 1000;
const int SERVERPORT = 6666;

class myepoll {
public:
    myepoll();
    ~myepoll();
private:
    int socketfd;
    struct sockaddr_in serveraddr;
    int epollfd;
    string err_msg;
public:
    int start();
    int get_socketfd() const;
    int do_epoll();

    void add_event(int fd, int state) const;
    void del_event(int fd, int state) const;
    void mod_event(int fd, int state) const;

    void handle_events(struct epoll_event (&events)[MAXEVENTS], int num);
    int handle_accept();
    int do_read(int fd);
    int do_write(int fd);


private:
    void set_err_msg(const string msg);

};


#endif //EPOLLTEST_MYEPOLL_H
