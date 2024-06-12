#include "EPollPoller.h"
#include "../Base/Log.h"
#include <fcntl.h>

EPollPoller::EPollPoller()
{
    epollfd = epoll_create(100);

}

EPollPoller::~EPollPoller()
{

}

EPollPoller* EPollPoller::createNew()                           
{
    return new EPollPoller();                                   // 创建一个新的 EPollPoller 实例并返回指针
    //    return New<SelectPoller>::allocate();
}

bool EPollPoller::addIOEvent(IOEvent* event)                    // 向 epoll 中添加一个 IOEvent    
{
    return updateIOEvent(event);
}

bool EPollPoller::updateIOEvent(IOEvent* event)                 // 添加或更新一个 IOEvent
{
    int fd = event->getFd();
    if (fd < 0)
    {
        LOGE("fd=%d", fd);                                      // 日志：文件描述符无效

        return false;
    }

    Read_events.data.fd = fd;                                   // 设置 epoll 事件类型为读事件
    Read_events.events = EPOLLIN;

    IOEventMap::iterator it = mEventMap.find(fd);               // 查找 IOEvent 是否已经存在于事件映射表中
    if (it != mEventMap.end())                              //先前已经添加则修改  存在
    {
        epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &Read_events);
    }
    else                                                    //先前未添加则添加IO事件  不存在
    {
        epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &Read_events);
        mEventMap.insert(std::make_pair(fd, event));            // 将事件添加到事件映射表中
    }

    if (mEventMap.empty())                          // 更新最大文件描述符
        mMaxNumSockets = 0;                             //如果 mEventMap 为空，则将 mMaxNumSockets 设置为 0，表示没有任何事件存在
    else
        mMaxNumSockets = mEventMap.rbegin()->first + 1; //如果 mEventMap 不为空，则获取事件映射表中最后一个元素，即最大的文件描述符，并加 1，以保证 mMaxNumSockets 表示的是可用的文件描述符数量。
                                                        //更新最大文件描述符+1（map会自动排序）最后一位

    return true;
}

bool EPollPoller::removeIOEvent(IOEvent* event)             // 从 epoll 中移除一个 IOEvent
{
    int fd = event->getFd();
    if (fd < 0)
        return false;


    IOEventMap::iterator it = mEventMap.find(fd);               // 从事件映射表中移除事件
    if (it != mEventMap.end())
        mEventMap.erase(it);

    if (mEventMap.empty())                                  // 更新最大文件描述符
        mMaxNumSockets = 0;
    else
        mMaxNumSockets = mEventMap.rbegin()->first + 1;

    return true;
}

void EPollPoller::handleEvent() {                                        // 处理事件

    epoll_event events[mMaxNumSockets];
    int ret;
    int rEvent;

    ret = epoll_wait(epollfd, events, mMaxNumSockets, -1);              // 等待监听的所有fd相应事件的产生（成功返回需要处理的事件数目。失败返回0，表示等待超时）
    //epollfd:epoll实例的句柄    events:epoll把发生的事件的集合从内核复制到 events数组中          mMaxNumSockets:本次可以返回的最大事件数目   -1:没有检测到事件发生时最多等待的时间（阻塞）

    if (ret < 0) {                                                  // 如果返回值小于 0，则发生错误
        return;
    }
    else {
    }

    for (int i = 0; i < ret; i++)                                       // 把监听到的io事件全部存到mIOEvent里面
    {
        int sockfd = events[i].data.fd;
        mEventMap[sockfd]->setREvent(IOEvent::EVENT_READ);              // 设置事件类型为读事件
        mIOEvents.push_back(mEventMap[sockfd]);                         // 将事件添加到事件列表中
    }

    for (auto& ioEvent : mIOEvents) {                               // 处理事件列表中的每个事件
        ioEvent->handleEvent();                                 //←此处进行处理readCallback
    }

    mIOEvents.clear();                                          // 清空事件列表

}