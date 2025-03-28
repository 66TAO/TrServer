﻿#ifndef BXC_RTSPSERVER_THREAD_H
#define BXC_RTSPSERVER_THREAD_H
#include <thread>
#include "../Base/Log.h"

class Thread
{
public:
    virtual ~Thread();
    
    bool start(void *arg);
    bool detach();
    bool join();

protected:
    Thread();

    virtual void run(void *arg) = 0;

private:
    static void* threadRun(void*);

private:
    void *mArg;
    bool mIsStart;
    bool mIsDetach;
    std::thread mThreadId;
};

#endif //BXC_RTSPSERVER_THREAD_H

