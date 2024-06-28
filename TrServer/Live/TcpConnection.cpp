#include "TcpConnection.h"
#include "../Scheduler/SocketsOps.h"
#include "../Base/Log.h"
#include "../Scheduler/EventScheduler.h"


TcpConnection::TcpConnection(UsageEnvironment* env, int clientFd) :
    mEnv(env),
    mClientFd(clientFd)
{
    mClientIOEvent = IOEvent::createNew(clientFd, this);                    // 创建一个新的 IOEvent 对象来管理客户端文件描述符的 I/O 事件
    mClientIOEvent->setReadCallback(readCallback);                          //←主要处理database// 设置读、写和错误的回调函数
    mClientIOEvent->setWriteCallback(writeCallback);
    mClientIOEvent->setErrorCallback(errorCallback);
    mClientIOEvent->enableReadHandling(); //默认只开启读

    mClienttimeEvent = TimerEvent::createNew(this);
    mClienttimeEvent->setTimeoutCallback(timeOutdisconnectCallback);
    //mClienttimeEvent->start();
    mtimeid = mEnv->scheduler()->addTimerEventRunEvery(mClienttimeEvent, 15 * 60 * 1000);//发送22包的定时间隔为10分钟10 * 60 * 1000


    //    mClientIOEvent->enableWriteHandling();
    //    mClientIOEvent->enableErrorHandling();

    mEnv->scheduler()->addIOEvent(mClientIOEvent);//加入客户端IO事件           // 将客户端 IOEvent 添加到调度器中，以便后续事件处理
}

TcpConnection::~TcpConnection()
{
    mEnv->scheduler()->removeIOEvent(mClientIOEvent);                   // 从调度器中移除客户端 IOEvent
    mEnv->scheduler()->removeTimerEventRunEvery(mtimeid);
    delete mClientIOEvent;                                              // 删除客户端 IOEvent 对象
    delete mClienttimeEvent;
    //    Delete::release(mClientIOEvent);

    sockets::close(mClientFd);                                          // 关闭客户端文件描述符
}

void TcpConnection::setDisConnectCallback(DisConnectCallback cb, void* arg)         // 设置断开连接回调函数及其参数
{
    mDisConnectCallback = cb;
    mArg = arg;
}

void TcpConnection::enableReadHandling()                                // 启用读取处理
{
    if (mClientIOEvent->isReadHandling())                               // 如果已经开启了读取处理，则直接返回
        return;

    mClientIOEvent->enableReadHandling();                               // 开启读取处理
    mEnv->scheduler()->updateIOEvent(mClientIOEvent);                   // 更新客户端 IOEvent 的事件状态到调度器中
}

void TcpConnection::enableWriteHandling()
{
    if (mClientIOEvent->isWriteHandling())
        return;

    mClientIOEvent->enableWriteHandling();
    mEnv->scheduler()->updateIOEvent(mClientIOEvent);
}

void TcpConnection::enableErrorHandling()
{
    if (mClientIOEvent->isErrorHandling())
        return;

    mClientIOEvent->enableErrorHandling();
    mEnv->scheduler()->updateIOEvent(mClientIOEvent);
}

void TcpConnection::disableReadeHandling()                          // 禁用读取处理
{
    if (!mClientIOEvent->isReadHandling())                          // 如果未开启读取处理，则直接返回
        return;

    mClientIOEvent->disableReadeHandling();                         // 禁用读取处理
    mEnv->scheduler()->updateIOEvent(mClientIOEvent);               // 更新客户端 IOEvent 的事件状态到调度器中
}

void TcpConnection::disableWriteHandling()
{
    if (!mClientIOEvent->isWriteHandling())
        return;

    mClientIOEvent->disableWriteHandling();
    mEnv->scheduler()->updateIOEvent(mClientIOEvent);
}

void TcpConnection::disableErrorHandling()
{
    if (!mClientIOEvent->isErrorHandling())
        return;

    mClientIOEvent->disableErrorHandling();
    mEnv->scheduler()->updateIOEvent(mClientIOEvent);
}

void TcpConnection::handleRead() {                                  // 处理读取事件

    //LOGI("");
    int ret = mInputBuffer.read(mClientFd);                         // 从客户端读取数据到输入缓冲区

    if (ret <= 0)                                                   // 如果读取出错或者读取的字节数小于等于 0，则认为客户端已经断开连接
    {
        for (const auto& pair : device_match) {
            if (pair.second == mClientFd) {
                string ISR_id = pair.first;
                LOGE("read error,fd=%d,ret=%d,ISRid=%s", mClientFd, ret, ISR_id.c_str());
                handleDisConnect();                                         // 处理断开连接事件
                return;
            }
        }
        LOGE("read error,fd=%d,ret=%d,no ISRid", mClientFd, ret);
        handleDisConnect();                                         // 处理断开连接事件
        return;
    }

    /* 先取消读 */
    //this->disableReadeHandling();

    handleReadBytes();// 调用RtspConnecton对象的实现函数             // 处理读取到的数据         ←主要处理数据函数
}

void TcpConnection::handleReadBytes() {//TrConnection重写该方法       子类重写
    LOGI("");
}
void TcpConnection::handleDisConnect()
{
    if (mDisConnectCallback) {                                      // 如果设置了断开连接回调函数，则调用该回调函数
        mDisConnectCallback(mArg, mClientFd);
    }
}
void TcpConnection::handleWrite()
{
    LOGI("");

}

void TcpConnection::handleError()
{
    LOGI("");
}

void TcpConnection::readCallback(void* arg)                         //读的回调函数
{
    TcpConnection* tcpConnection = (TcpConnection*)arg;
    tcpConnection->handleRead();                                        //←主要数据处理
}

void TcpConnection::writeCallback(void* arg)
{
    TcpConnection* tcpConnection = (TcpConnection*)arg;
    tcpConnection->handleWrite();
}

void TcpConnection::errorCallback(void* arg)
{
    TcpConnection* tcpConnection = (TcpConnection*)arg;
    tcpConnection->handleError();
}

void TcpConnection::timeOutdisconnectCallback(void* arg) {
    TcpConnection* timeOutdisconnect = (TcpConnection*)arg;
    for (const auto& pair : device_match) {
        if (pair.second == timeOutdisconnect->mClientFd) {
            string ISR_id = pair.first;
            LOGE("time out, will close the fd=%d,timeid=%d,ISRid=%s", timeOutdisconnect->mClientFd, timeOutdisconnect->mtimeid, ISR_id.c_str());
            timeOutdisconnect->handleDisConnect();                                         // 处理断开连接事件
            return;
        }
    }
    LOGI("time out, will close the fd=%d,timeid=%d, no ISRid", timeOutdisconnect->mClientFd, timeOutdisconnect->mtimeid);
    //cout << "time out will close the fd:" << timeOutdisconnect->mClientFd << endl;
    //cout << "time out will close the timefd:" << timeOutdisconnect->mtimeid << endl;
    timeOutdisconnect->handleDisConnect();
}

