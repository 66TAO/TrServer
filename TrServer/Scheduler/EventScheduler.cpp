#include "EventScheduler.h"
#include "SocketsOps.h"
#include "SelectPoller.h"
//#include "PollPoller.h"
#include "EPollPoller.h"
#include "../Base/Log.h"
#include <iostream>

#include <sys/eventfd.h>

EventScheduler* EventScheduler::createNew(PollerType type)                      //参数type表示轮询器的类型，可以是POLLER_SELECT、POLLER_POLL或POLLER_EPOLL
{
    if (type != POLLER_SELECT && type != POLLER_POLL && type != POLLER_EPOLL)               // 检查轮询器类型是否合法
        return NULL;

    return new EventScheduler(type);                                    // 创建并返回一个新的EventScheduler对象
}

EventScheduler::EventScheduler(PollerType type) : mQuit(false) {


    switch (type) {                                         // 根据轮询器类型选择合适的轮询器

    case POLLER_SELECT:
        mPoller = SelectPoller::createNew();
        break;

        //case POLLER_POLL:
        //    mPoller = PollPoller::createNew();
        //    break;

    case POLLER_EPOLL:
        mPoller = EPollPoller::createNew();
        break;

    default:                                            // 如果轮询器类型不合法，则直接退出程序
        _exit(-1);
        break;
    }

    mTimerManager = TimerManager::createNew(this);
}

EventScheduler::~EventScheduler()                       // EventScheduler类的析构函数，释放轮询器对象的内存
{

    delete mPoller;

}

bool EventScheduler::addTriggerEvent(TriggerEvent* event)               // 向事件调度器添加一个触发器事件
{
    mTriggerEvents.push_back(event);                                // 将事件添加到触发器事件列表中

    return true;
}

bool EventScheduler::addIOEvent(IOEvent* event)                         // 向事件调度器添加一个IO事件
{
    return mPoller->addIOEvent(event);
}

bool EventScheduler::updateIOEvent(IOEvent* event)                      // 更新事件调度器中的一个IO事件
{
    return mPoller->updateIOEvent(event);
}

bool EventScheduler::removeIOEvent(IOEvent* event)                      // 从事件调度器中移除一个IO事件
{
    return mPoller->removeIOEvent(event);
}

void EventScheduler::loop() {                                       // 事件调度器的主循环，处理触发器事件并调用轮询器处理IO事件
    while (!mQuit) {
        handleTriggerEvents();                                      //处理TriggerEvents
        mPoller->handleEvent();                                     //处理IOEvents    //epoll->handleEvent
    }
}

void EventScheduler::handleTriggerEvents()                                               // 处理触发器事件的方法
{
    if (!mTriggerEvents.empty())                                                        // 如果触发器事件列表不为空
    {
        for (std::vector<TriggerEvent*>::iterator it = mTriggerEvents.begin();          // 遍历触发器事件列表，依次处理每个事件
            it != mTriggerEvents.end(); ++it)
        {
            (*it)->handleEvent();                                           //TriggerEvent->hendleEvent
        }

        mTriggerEvents.clear();                                                             // 清空触发器事件列表
    }
}

Poller* EventScheduler::poller() {                                                  // 返回事件调度器中使用的轮询器对象
    return mPoller;
}

Timer::TimerId EventScheduler::addTimerEventRunEvery(TimerEvent* event, Timer::TimeInterval interval)
{
    //std::cout << "add timerEvent " << event->getName() << " run every time, Interval = " << interval / 1000 << "s" << std::endl;
    Timer::TimeStamp timeStamp = Timer::getCurTime();
    timeStamp += interval;
    return mTimerManager->addTimer(event, timeStamp, interval);
}

Timer::TimerId EventScheduler::resetTimerEvent(Timer::TimerId mtimeid, Timer::TimeInterval interval)
{
    std::cout << "rest timerEvent " << mtimeid << "  Interval = " << interval / 1000 << "s" << std::endl;
    return mTimerManager->resetTimer(mtimeid, interval);
}