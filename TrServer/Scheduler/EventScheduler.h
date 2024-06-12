#pragma once

#include <vector>
#include <queue>
#include <mutex>
#include <stdint.h>
#include "Event.h"
class Poller;

class EventScheduler
{
public:
    enum PollerType
    {
        POLLER_SELECT,
        POLLER_POLL,
        POLLER_EPOLL
    };
    static EventScheduler* createNew(PollerType type);

    explicit EventScheduler(PollerType type);
    virtual ~EventScheduler();
public:
    bool addTriggerEvent(TriggerEvent* event);
    bool addIOEvent(IOEvent* event);
    bool updateIOEvent(IOEvent* event);
    bool removeIOEvent(IOEvent* event);

    void loop();
    //    void wakeup();
    Poller* poller();

private:
    void handleTriggerEvents();

private:
    bool mQuit;
    Poller* mPoller;
    std::vector<TriggerEvent*> mTriggerEvents;
};

