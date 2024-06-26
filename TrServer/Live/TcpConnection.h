#pragma once
#include "../Scheduler/UsageEnvironment.h"
#include "../Scheduler/Event.h"
#include "Buffer.h"
#include "TrServer.h"
#include "../Scheduler/Timer.h"

class TrServer;
class TcpConnection
{
public:
    typedef void (*DisConnectCallback)(void*, int);

    TcpConnection(UsageEnvironment* env, int clientFd);
    TcpConnection() {};
    virtual ~TcpConnection();

    void setDisConnectCallback(DisConnectCallback cb, void* arg);

protected:
    void enableReadHandling();
    void enableWriteHandling();
    void enableErrorHandling();
    void disableReadeHandling();
    void disableWriteHandling();
    void disableErrorHandling();

    void handleRead();
    virtual void handleReadBytes();
    virtual void handleWrite();
    virtual void handleError();

    void handleDisConnect();

private:
    static void readCallback(void* arg);
    static void writeCallback(void* arg);
    static void errorCallback(void* arg);
    static void timeOutdisconnectCallback(void* arg);

protected:
    UsageEnvironment* mEnv;
    int mClientFd;
    Timer::TimerId mtimeid;
    IOEvent* mClientIOEvent;
    TimerEvent* mClienttimeEvent;
    DisConnectCallback mDisConnectCallback;//在TrServer实例化该类子类的实例时，设置的回调函数
    void* mArg;
    Buffer mInputBuffer;
    char mBuffer[2048];
};
