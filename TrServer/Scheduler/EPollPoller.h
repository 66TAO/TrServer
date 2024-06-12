#pragma once
#include "Poller.h"
#include <vector>

#include <sys/epoll.h>

class EPollPoller : public Poller
{
public:
    EPollPoller();
    virtual ~EPollPoller();

    static EPollPoller* createNew();

    virtual bool addIOEvent(IOEvent* event);
    virtual bool updateIOEvent(IOEvent* event);
    virtual bool removeIOEvent(IOEvent* event);
    virtual void handleEvent();

private:
    epoll_event Read_events;
    int epollfd;                //epoll实例
    int mMaxNumSockets;
    std::vector<IOEvent*> mIOEvents;// 存储临时活跃的IO事件对象
};