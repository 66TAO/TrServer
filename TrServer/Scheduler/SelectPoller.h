#pragma once
#include "Poller.h"
#include <vector>

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>


class SelectPoller : public Poller
{
public:
    SelectPoller();
    virtual ~SelectPoller();

    static SelectPoller* createNew();

    virtual bool addIOEvent(IOEvent* event);
    virtual bool updateIOEvent(IOEvent* event);
    virtual bool removeIOEvent(IOEvent* event);
    virtual void handleEvent();

private:
    fd_set mReadSet;
    fd_set mWriteSet;
    fd_set mExceptionSet;
    int mMaxNumSockets;
    std::vector<IOEvent*> mIOEvents;// 存储临时活跃的IO事件对象

};
