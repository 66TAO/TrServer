#include "Scheduler/EventScheduler.h"
#include "Scheduler/UsageEnvironment.h"
#include "Live/InetAddress.h"
#include "Live/TrServer.h"
#include "Live/TrDatabase.h"



int main()
{

	TrDatabase* Trdb = TrDatabase::createNew();		
	ThreadPool* threadPool = ThreadPool::createNew(4);
	EventScheduler* scheduler = EventScheduler::createNew(EventScheduler::POLLER_EPOLL);
	UsageEnvironment* env = UsageEnvironment::createNew(scheduler, threadPool);

	Ipv4Address rtspAddr("0.0.0.0", 8888);											
	TrServer* TrServer = TrServer::createNew(env, rtspAddr, Trdb);				
	//setReadCallback(readCallback) -> TrServer::readCallback -> TrServer* handleRead -> TrConnection::createNew -> TcpConnection::createNew -> setReadCallback(readCallback) -> TcpConnection::readCallback -> addIOEvent(mClientIOEvent) -> TcpConnection::handleReadBytes() ->  TrConnection::handleReadBytes() 
	TrServer->start();								

	env->scheduler()->loop();						

	return 0;

}