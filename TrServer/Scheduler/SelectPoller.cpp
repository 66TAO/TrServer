#include "SelectPoller.h"
#include "../Base/Log.h"

SelectPoller::SelectPoller()
{
    FD_ZERO(&mReadSet);                                             // 清空读文件描述符集合
    FD_ZERO(&mWriteSet);                                            // 清空写文件描述符集合
    FD_ZERO(&mExceptionSet);                                        // 清空异常文件描述符集合
}

SelectPoller::~SelectPoller()
{

}

SelectPoller* SelectPoller::createNew()                             // SelectPoller类的静态方法，用于创建一个新的SelectPoller对象
{
    return new SelectPoller();                                      // 返回一个新的SelectPoller对象
    //    return New<SelectPoller>::allocate();
}

bool SelectPoller::addIOEvent(IOEvent* event)                       // 添加一个IO事件到轮询器中
{
    return updateIOEvent(event);                                    // 调用更新IO事件的方法
}

bool SelectPoller::updateIOEvent(IOEvent* event)                    // 更新轮询器中的一个IO事件
{
    int fd = event->getFd();                                        // 获取事件的文件描述符
    if (fd < 0)
    {
        LOGE("fd=%d", fd);                                          // 输出错误日志

        return false;
    }

    FD_CLR(fd, &mReadSet);                                          // 清除文件描述符对应的事件集合
    FD_CLR(fd, &mWriteSet);
    FD_CLR(fd, &mExceptionSet);

    IOEventMap::iterator it = mEventMap.find(fd);                   // 查找事件是否已经添加到事件映射中
    if (it != mEventMap.end())                                      //先前已经添加则修改  存在     // 根据事件类型设置相应的事件集合
    {
        if (event->isReadHandling())
            FD_SET(fd, &mReadSet);
        if (event->isWriteHandling())
            FD_SET(fd, &mWriteSet);
        if (event->isErrorHandling())
            FD_SET(fd, &mExceptionSet);
    }
    else                                                    //先前未添加则添加IO事件  不存在     // 根据事件类型设置相应的事件集合
    {
        if (event->isReadHandling())
            FD_SET(fd, &mReadSet);
        if (event->isWriteHandling())
            FD_SET(fd, &mWriteSet);
        if (event->isErrorHandling())
            FD_SET(fd, &mExceptionSet);

        mEventMap.insert(std::make_pair(fd, event));                    // 将事件添加到事件映射中
    }

    if (mEventMap.empty())                                              // 更新最大文件描述符数目
        mMaxNumSockets = 0;
    else
        mMaxNumSockets = mEventMap.rbegin()->first + 1;                 //更新最大文件描述符+1（map会自动排序）最后一位

    return true;
}

bool SelectPoller::removeIOEvent(IOEvent* event)                            // 从轮询器中移除一个IO事件
{
    int fd = event->getFd();
    if (fd < 0)
        return false;

    FD_CLR(fd, &mReadSet);                                                  // 清除文件描述符对应的事件集合
    FD_CLR(fd, &mWriteSet);
    FD_CLR(fd, &mExceptionSet);

    IOEventMap::iterator it = mEventMap.find(fd);                           // 在事件映射中移除事件
    if (it != mEventMap.end())
        mEventMap.erase(it);

    if (mEventMap.empty())                                                  // 更新最大文件描述符数目
        mMaxNumSockets = 0;
    else
        mMaxNumSockets = mEventMap.rbegin()->first + 1;

    return true;
}

void SelectPoller::handleEvent() {                                          // 处理事件的方法

    fd_set readSet = mReadSet;
    fd_set writeSet = mWriteSet;
    fd_set exceptionSet = mExceptionSet;
    struct timeval timeout;
    int ret;
    int rEvent;

    timeout.tv_sec = 1000;// 秒
    timeout.tv_usec = 0;//微秒
    //LOGI("mEventMap.size() = %d，select io start", mEventMap.size());

    ret = select(mMaxNumSockets, &readSet, &writeSet, &exceptionSet, &timeout);                 // 调用select函数进行事件轮询


    if (ret < 0) {                                                          // 如果select函数返回值小于0，则表示发生错误，直接返回
        return;
    }
    else {
        //LOGI("检测到活跃的描述符 ret=%d", ret);
    }
    //LOGI("select io end");

    for (IOEventMap::iterator it = mEventMap.begin(); it != mEventMap.end(); ++it)          // 遍历事件映射，检查哪些文件描述符有事件发生
    {
        rEvent = 0;
        if (FD_ISSET(it->first, &readSet))                              // 检查读事件是否发生
            rEvent |= IOEvent::EVENT_READ;

        if (FD_ISSET(it->first, &writeSet))                             
            rEvent |= IOEvent::EVENT_WRITE;

        if (FD_ISSET(it->first, &exceptionSet))
            rEvent |= IOEvent::EVENT_ERROR;

        if (rEvent != 0)                                            // 如果有事件发生，则设置事件的就绪事件，并将事件加入待处理事件列表
        {
            it->second->setREvent(rEvent);
            mIOEvents.push_back(it->second);
        }
    }

    for (auto& ioEvent : mIOEvents) {                           // 处理待处理事件列表中的所有事件
        ioEvent->handleEvent();
    }

    mIOEvents.clear();                                      // 清空待处理事件列表

}



