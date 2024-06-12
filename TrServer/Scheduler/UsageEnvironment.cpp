#include "UsageEnvironment.h"

UsageEnvironment* UsageEnvironment::createNew(EventScheduler* scheduler)
{
    return new UsageEnvironment(scheduler);
}

UsageEnvironment::UsageEnvironment(EventScheduler* scheduler) :
    mScheduler(scheduler)
{

}

UsageEnvironment::~UsageEnvironment()
{

}

EventScheduler* UsageEnvironment::scheduler()
{
    return mScheduler;
}