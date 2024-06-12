#include "Scheduler/EventScheduler.h"
#include "Scheduler/UsageEnvironment.h"
#include "Live/InetAddress.h"
#include "Live/TrServer.h"
#include "Live/TrDatabase.h"



int main()
{

	TrDatabase* Trdb = TrDatabase::createNew();												//创建TrDatabase类没过多操作
	EventScheduler* scheduler = EventScheduler::createNew(EventScheduler::POLLER_EPOLL);		//创建一个调度器类选择使用epoll，创建一个容量为100的池子
	UsageEnvironment* env = UsageEnvironment::createNew(scheduler);						//创建一个新的环境将scheduler赋值给mScheduler

	Ipv4Address rtspAddr("0.0.0.0", 8888);										//创建一个IPv4地址类并设置IP和端口号	端口号为8888	
	//Ipv4Address rtspAddr("0.0.0.0", 12345);
	TrServer* TrServer = TrServer::createNew(env, rtspAddr, Trdb);				//创建一个服务器类mEnv(env),mAddr(rtspAddr),Trdb(Trdb),配置mAcceptIOEvent和mCloseTriggerEvent
	//setReadCallback(readCallback) -> TrServer::readCallback -> TrServer* handleRead -> TrConnection::createNew -> TcpConnection::createNew -> setReadCallback(readCallback) -> TcpConnection::readCallback -> addIOEvent(mClientIOEvent) -> TcpConnection::handleReadBytes() ->  TrConnection::handleReadBytes() 
	TrServer->start();								//创建一个send_message的线程，将mAcceptIOEvent加入到调度器中，并初始化数据库

	env->scheduler()->loop();						//将调度器中的事件挨个处理

	return 0;

}