//
// Created by on 2020/8/9.
//

#include <sstream>
#include "myepoll.h"
using namespace std;

myepoll::myepoll()
{
    socketfd = 0;
    epollfd = 0;
    memset(&serveraddr, 0, sizeof(serveraddr));
}
myepoll::~myepoll()
{
    if (socketfd)
        close(socketfd);
    if (epollfd)
        close(epollfd);
}

void myepoll::set_err_msg(const string msg)
{
    ostringstream ostrStream;
    ostrStream.clear();
    ostrStream << msg << strerror(errno) << "(errno:" << errno << ")" << endl;
    err_msg = ostrStream.str();
}

int myepoll::start()
{
    int res = 0;

    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd < 0)
    {
        set_err_msg("create socket error: ");
        return -1;
    }
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = SERVERPORT;

    int flags = fcntl(socketfd, F_GETFL, 0);
    fcntl(socketfd, F_SETFL, flags|O_NONBLOCK);

    //地址复用
    int on = 1;

    res = setsockopt(socketfd,SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if (res == -1)
    {
        set_err_msg("set reuse addr error: ");
        return -1;
    }

    //绑定socket
    res = bind(socketfd, (struct sockaddr*) &serveraddr, sizeof(serveraddr));
    if (res == -1)
    {
        set_err_msg("bind socket error: ");
        return -1;
    }

    //开始listen
    res = listen(socketfd, 5);
    if (res == -1)
    {
        set_err_msg("listen socket error: ");
        return -1;
    }
    cout << "socket listen success! " << endl;
    return 0;
}

int myepoll::get_socketfd() const
{
    return socketfd;
}

int myepoll::do_epoll()
{
    struct epoll_event events[MAXEVENTS];
    int ret = 0;
    char buf[MAXBUFFSIXE] = {0};
    int buflen = 0;

    epollfd = epoll_create(FDSIZE);
    if (epollfd == -1)
    {
        set_err_msg("create epoll error: ");
        return -1;
    }

    //将服务端socketfd添加到epoll监听
    add_event(socketfd, EPOLLIN);
    cout << "add listen socketfd to epoll success!" << endl;
    while(true)
    {
        ret = epoll_wait(epollfd, events, MAXEVENTS, 500);
        cout << "epoll wait ret: " << ret <<endl;
        if (ret <= 0)
        {
            sleep(1);
        }
        else
        {
            handle_events(events, ret);
        }
    }
    close(epollfd);
}


void myepoll::handle_events(struct epoll_event (&events)[MAXEVENTS], int num)
{
    int i = 0;
    int fd = 0;

    for (i = 0; i < num; i++)
    {
        fd = events[i].data.fd;
        if ((fd == socketfd) && ((events[i].events & EPOLLIN) == EPOLLIN))
        {
            cout << "get client accept!" << endl;
            handle_accept();
        }
        else if ((events[i].events & EPOLLIN) == EPOLLIN)
        {
            cout << "get read!" << endl;

            do_read(fd);
        }
        else if ((events[i].events & EPOLLOUT) == EPOLLOUT)
        {
            do_write(fd);
        }
        else
        {
            close(fd);
        }
    }
}

void myepoll::add_event(int fd, int state) const
{
    struct epoll_event ev;
    ev.events = state;
    ev.events |= EPOLLET;
    ev.data.fd = fd;

    int flags = fcntl(fd, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(fd, F_SETFL, flags);
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
}

void myepoll::del_event(int fd, int state) const
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);
}

void myepoll::mod_event(int fd, int state) const
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev);
}

int myepoll::handle_accept()
{
    int clifd = 0;
    struct sockaddr_in cliaddr;
    socklen_t cliaddrlen = sizeof(cliaddr);
    clifd = accept(socketfd, (struct sockaddr*) &cliaddr, &cliaddrlen);
    if (clifd == -1)
    {
        set_err_msg("listen accept error: ");
        return -1;
    }
    else
    {
        cout << "accept a new socket client: " << inet_ntoa(cliaddr.sin_addr) << ":" << cliaddr.sin_port << endl;
        add_event(clifd, EPOLLIN);
        return 0;
    }

    return 0;
}

int myepoll::do_read(int fd)
{
    char buf[MAXBUFFSIXE] = {0};
    int bufLen = 0;
    bufLen = read(fd, buf, MAXBUFFSIXE);
    if (bufLen == -1)
    {
        set_err_msg("read error: ");
        close(fd);
        del_event(fd, EPOLLIN);
        return -1;
    }
    else if (bufLen == 0)
    {
        close(fd);
        cout << "client closed, fd:" << fd << endl;
        del_event(fd, EPOLLIN);
        return -1;
    }
    else
    {
        cout << "receive msg: " << buf << endl;
        mod_event(fd, EPOLLOUT);
        return 0;
    }
    return 0;
}

int myepoll::do_write(int fd)
{
    ostringstream ostrStream;
    ostrStream << time(NULL);
    string retStr = "response from server at : " + ostrStream.str() + " !";

    int bufLen = 0;
    int nwrite = 0;
    nwrite = write(fd, (char *)retStr.c_str(), retStr.length());
    if (nwrite == -1)
    {
        set_err_msg("write error: ");
        close(fd);
        del_event(fd, EPOLLOUT);
        return -1;
    }
    else
    {
        cout << "write msg: " << retStr << endl;
        mod_event(fd, EPOLLIN);
        return 0;
    }
    return 0;
}