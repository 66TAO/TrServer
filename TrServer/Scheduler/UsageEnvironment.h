#pragma once

#include "EventScheduler.h"

class UsageEnvironment
{
public:
    static UsageEnvironment* createNew(EventScheduler* scheduler);

    UsageEnvironment(EventScheduler* scheduler);
    ~UsageEnvironment();

    EventScheduler* scheduler();

private:
    EventScheduler* mScheduler;
};
