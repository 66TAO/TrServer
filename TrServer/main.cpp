#include "Scheduler/EventScheduler.h"
#include "Scheduler/UsageEnvironment.h"
#include "Live/InetAddress.h"
#include "Live/TrServer.h"
#include "Live/TrDatabase.h"



int main()
{

	TrDatabase* Trdb = TrDatabase::createNew();												//����TrDatabase��û�������
	EventScheduler* scheduler = EventScheduler::createNew(EventScheduler::POLLER_EPOLL);		//����һ����������ѡ��ʹ��epoll������һ������Ϊ100�ĳ���
	UsageEnvironment* env = UsageEnvironment::createNew(scheduler);						//����һ���µĻ�����scheduler��ֵ��mScheduler

	Ipv4Address rtspAddr("0.0.0.0", 8888);										//����һ��IPv4��ַ�ಢ����IP�Ͷ˿ں�	�˿ں�Ϊ8888	
	//Ipv4Address rtspAddr("0.0.0.0", 12345);
	TrServer* TrServer = TrServer::createNew(env, rtspAddr, Trdb);				//����һ����������mEnv(env),mAddr(rtspAddr),Trdb(Trdb),����mAcceptIOEvent��mCloseTriggerEvent
	//setReadCallback(readCallback) -> TrServer::readCallback -> TrServer* handleRead -> TrConnection::createNew -> TcpConnection::createNew -> setReadCallback(readCallback) -> TcpConnection::readCallback -> addIOEvent(mClientIOEvent) -> TcpConnection::handleReadBytes() ->  TrConnection::handleReadBytes() 
	TrServer->start();								//����һ��send_message���̣߳���mAcceptIOEvent���뵽�������У�����ʼ�����ݿ�

	env->scheduler()->loop();						//���������е��¼���������

	return 0;

}