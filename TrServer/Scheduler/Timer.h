#ifndef TRLIB_TIMER_H
#define TRLIB_TIMER_H
#include<map>
#include<stdint.h>

//���ǰ������
class EventScheduler;
class Poller;
class TimerEvent;
class IOEvent;

class Timer {	//��ʱ��
public:
	typedef uint32_t TimerId;	//��ʱ��id
	typedef int64_t TimeStamp;	//ʱ���
	typedef uint32_t TimeInterval;	//ʱ����

	Timer();
	~Timer();

	static TimeStamp getCurTime();	//��̬��Ա����������������ֱ�ӵ��ã����ô�������
	static TimeStamp getCurTimeStamp();

private:
	friend class TimerManager;	//��Ԫ�࣬���Է���Timer��˽�г�Ա
	Timer(TimerEvent* event, TimeStamp stamp, TimeInterval interval, TimerId timeId);

private:
	bool handleEvent();

private:
	TimerEvent* mTimerEvent;	//��ʱ���¼�
	TimeStamp mTimeStamp;
	TimeInterval mTimeInterval;
	TimerId mTimerId;
	bool mRepeat;	//�Ƿ��ظ�
};

class TimerManager {	//��ʱ��������
public:
	static TimerManager* createNew(EventScheduler* scheduler);
	TimerManager(EventScheduler* scheduler);
	~TimerManager();

	Timer::TimerId addTimer(TimerEvent* event, Timer::TimeStamp timeStamp, Timer::TimeInterval timeInterval);
	Timer::TimerId resetTimer(Timer::TimerId timerId, Timer::TimeInterval timeInterval);
	bool removeTimer(Timer::TimerId timeId);

private:
	static void readCallback(void* arg);
	void handleRead();
	void modifyTimeout();

public:
	Poller* mPoller;
	std::map<Timer::TimerId, Timer> mTimers;
	std::multimap<Timer::TimeStamp, Timer> mEvents;
	Timer::TimerId mLastTimerId;
	int mTimerFd;
	IOEvent* mTimerIOEvent;
};


#endif // !TRLIB_TIMER_H
