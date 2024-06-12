#include "TrServer.h"
#include "TrConnection.h"
#include "../Base/Log.h"
#include "../Scheduler/SocketsOps.h"
#include <thread>

TrServer* TrServer::createNew(UsageEnvironment* env, Ipv4Address& addr, TrDatabase* trdb) {

    return new TrServer(env, addr, trdb);
}
TrServer::TrServer(UsageEnvironment* env, Ipv4Address& addr, TrDatabase* trdb) :
    mEnv(env),
    mAddr(addr),
    Trdb(trdb),
    mListen(false)
{

    mFd = sockets::createTcpSock();															//��������������
    sockets::setReuseAddr(mFd, 1);															//�����׽���ѡ����ö˿ڣ�������ʱ���ܶ˿ڲ�δ�ͷţ����Ϻ���Լ�ʱ���ö˿ڣ�
    if (!sockets::bind(mFd, addr.getIp(), mAddr.getPort())) {
        return;
    }

    LOGI("rtsp://%s:%d fd=%d", addr.getIp().data(), addr.getPort(), mFd);					//����˵�ip���˿ڵ�ǰ�׽���

    mAcceptIOEvent = IOEvent::createNew(mFd, this);
    mAcceptIOEvent->setReadCallback(readCallback);											//����Ҫ���ݴ���//���ûص���socket�ɶ� ����ָ��˿ڽ��������¼�
    mAcceptIOEvent->enableReadHandling();													//����Ϊ�ɶ�

    mCloseTriggerEvent = TriggerEvent::createNew(this);
    mCloseTriggerEvent->setTriggerCallback(cbCloseConnect);									//���ûص��Ĺر����� ����ָ��

}

TrServer::~TrServer()
{
    if (mListen)
        mEnv->scheduler()->removeIOEvent(mAcceptIOEvent);

    delete mAcceptIOEvent;
    delete mCloseTriggerEvent;

    sockets::close(mFd);
}

char* TrServer::protocol_pack_mach_open()												// ����������"open"Э����ַ���
{
	char* res_pack = (char*)malloc(10);													//malloc������ 10 ���ֽڵ��ڴ�ռ䣬�������ַ�����ַ�ָ����� res_pack(�ɼ��ж����Ƿ񴴽��ɹ�)
	if (res_pack == nullptr) {
		cerr << "Memory allocation failed!" << endl;
		exit(EXIT_FAILURE);
	}
	strcpy(res_pack, "open");															//�� "open" ���Ƶ��� res_pack ��ָ����ڴ�ռ���
	return res_pack; 
}

char* TrServer::protocol_pack_mach_close()												// ����������"close"Э����ַ���
{
	char* res_pack = (char*)malloc(10); 
	if (res_pack == nullptr) {
		cerr << "Memory allocation failed!" << endl;
		exit(EXIT_FAILURE);
	}
	strcpy(res_pack, "close"); 
	return res_pack; 
}

void TrServer::thread_process_send_message() {
	char b;
	char* TS_buf = protocol_pack_mach_open();												// ��ȡ"open"Э����ַ���
	char* TS_buf1 = protocol_pack_mach_close();												// ��ȡ"close"Э����ַ���
	string key1 = "ISR0400000000004";
	string key2 = "ISR0112912310101";
	string key3 = "ISR1500000000015";
	string key4 = "ISR1100000000011";
	while (1) {
		while (cin >> b) {
			switch (b) {
			case '1':
				if (device_match.count(key1) > 0) {
					send(device_match[key1], TS_buf, strlen(TS_buf), MSG_DONTWAIT);//mes_fd�׽����ļ���������strlen(TS_buf)ָ�����͵��ֽ������ȣ�MSG_DONTWAIT������ģʽ����
					cout << "Reply 04 open successfully!!" << endl;
				}
				else {
					cout << "ISR04 be not online" << endl;
				}
				break;
			case '2':
				if (device_match.count(key1) > 0) {
					send(device_match[key1], TS_buf1, strlen(TS_buf1), MSG_DONTWAIT);//mes_fd�׽����ļ���������strlen(TS_buf)ָ�����͵��ֽ������ȣ�MSG_DONTWAIT������ģʽ����
					cout << "Reply 04 close successfully!!" << endl;
				}
				else {
					cout << "ISR04 be not online" << endl;
				}
				break;
			case '3':
				if (device_match.count(key2) > 0) {
					send(device_match[key2], TS_buf, strlen(TS_buf), MSG_DONTWAIT);//mes_fd�׽����ļ���������strlen(TS_buf)ָ�����͵��ֽ������ȣ�MSG_DONTWAIT������ģʽ����
					cout << "Reply 01 message successfully!!" << endl;
				}
				else {
					cout << "ISR01 be not online" << endl;
				}
				break;
			case '4':
				if (device_match.count(key2) > 0) {
					send(device_match[key2], TS_buf1, strlen(TS_buf1), MSG_DONTWAIT);//mes_fd�׽����ļ���������strlen(TS_buf)ָ�����͵��ֽ������ȣ�MSG_DONTWAIT������ģʽ����
					cout << "Reply 01 close successfully!!" << endl;
				}
				else {
					cout << "ISR01 be not online" << endl;
				}
				break;
			case '5':
				if (device_match.count(key3) > 0) {
					send(device_match[key3], TS_buf, strlen(TS_buf), MSG_DONTWAIT);//mes_fd�׽����ļ���������strlen(TS_buf)ָ�����͵��ֽ������ȣ�MSG_DONTWAIT������ģʽ����
					cout << "Reply 15 message successfully!!" << endl;
				}
				else {
					cout << "ISR15 be not online" << endl;
				}
				break;
			case '6':
				if (device_match.count(key3) > 0) {
					send(device_match[key3], TS_buf1, strlen(TS_buf1), MSG_DONTWAIT);//mes_fd�׽����ļ���������strlen(TS_buf)ָ�����͵��ֽ������ȣ�MSG_DONTWAIT������ģʽ����
					cout << "Reply 15 close successfully!!" << endl;
				}
				else {
					cout << "ISR15 be not online" << endl;
				}
				break;
			case '7':
				if (device_match.count(key4) > 0) {
					send(device_match[key4], TS_buf, strlen(TS_buf), MSG_DONTWAIT);//mes_fd�׽����ļ���������strlen(TS_buf)ָ�����͵��ֽ������ȣ�MSG_DONTWAIT������ģʽ����
					cout << "Reply 11 message successfully!!" << endl;
				}
				else {
					cout << "ISR11 be not online" << endl;
				}
				break;
			case '8':
				if (device_match.count(key4) > 0) {
					send(device_match[key4], TS_buf1, strlen(TS_buf1), MSG_DONTWAIT);//mes_fd�׽����ļ���������strlen(TS_buf)ָ�����͵��ֽ������ȣ�MSG_DONTWAIT������ģʽ����
					cout << "Reply 11 close successfully!!" << endl;
				}
				else {
					cout << "ISR11 be not online" << endl;
				}
				break;
			case '9':
				for (auto b : device_match) {
					cout << b.first << " " << b.second << endl;							// ��ӡ�豸ӳ���ϵ
				}
				break;

			default:
				cout << "Interface not open" << endl;
			}
		}
	}
	if (TS_buf != nullptr) {
		free(TS_buf);
	}
	if (TS_buf1 != nullptr) {
		free(TS_buf1);
	}
}

void TrServer::remove_by_value(std::map<std::string, int>& map, const int& value)				// �Ƴ�map���ض���ֵ
{
	for (auto it = map.begin(); it != map.end(); )
	{
		if (it->second == value)
		{
			it = map.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void TrServer::start() {															// ����������
    LOGI("Start thread");												
	std::thread t1(&TrServer::thread_process_send_message, this);		// ����һ���߳�ִ����Ϣ��������
	t1.detach();														// �����̣߳�ʹ���ں�̨����
    mListen = true;														// ��Ƿ���������״̬Ϊtrue
    sockets::listen(mFd, 60);											// ��ʼ�����ͻ�����������backlog����Ϊ60��������Ӹ�����
    mEnv->scheduler()->addIOEvent(mAcceptIOEvent);						// ��Accept�¼���ӵ��¼���������
    Trdb->init_db();													// ��ʼ�����ݿ�
}

void TrServer::readCallback(void* arg) {								// ���ж������Ļص�����
    TrServer* trServer = (TrServer*)arg;
    trServer->handleRead();												//����Ҫ�����ݴ���
		
}

void TrServer::handleRead() {											//����mAcceptIOEvent// ������¼�
    int clientFd = sockets::accept(mFd);								// ���ܿͻ������ӣ���ȡ�ͻ����׽����ļ�������
    if (clientFd < 0)
    {
        LOGE("handleRead error,clientFd=%d", clientFd);
        return;
    }
    TrConnection* conn = TrConnection::createNew(this, Trdb, clientFd);		// �����µ����Ӷ���    ����Ҫ�����ݴ���
    conn->setDisConnectCallback(TrServer::cbDisConnect, this);				// �������ӶϿ��ص�����
    mConnMap.insert(std::make_pair(clientFd, conn));						// �����Ӷ����������ӳ�����

}
void TrServer::cbDisConnect(void* arg, int clientFd) {					// ���ӶϿ��Ļص�����
    TrServer* server = (TrServer*)arg;

    server->handleDisConnect(clientFd);
}

void TrServer::handleDisConnect(int clientFd) {								// �������ӶϿ��¼�

	remove_by_value(device_match, clientFd);							// ���豸ӳ������Ƴ��Ͽ����ӵ��豸

    mDisConnList.push_back(clientFd);									// ���Ͽ����ӵĿͻ����׽����ļ���������ӵ��Ͽ������б���

    mEnv->scheduler()->addTriggerEvent(mCloseTriggerEvent);				// ��ӹر����Ӵ����¼����¼���������

}

void TrServer::cbCloseConnect(void* arg) {							// �ر����ӵĻص�����
    TrServer* server = (TrServer*)arg;
    server->handleCloseConnect();
}
void TrServer::handleCloseConnect() {																// ����ر������¼�


    for (std::vector<int>::iterator it = mDisConnList.begin(); it != mDisConnList.end(); ++it) {

        int clientFd = *it;									//itָ��ļ�ֵ���еļ���ֵ��clientFd
        std::map<int, TrConnection*>::iterator _it = mConnMap.find(clientFd);
        assert(_it != mConnMap.end());						//���ж��Լ����ָ���Ƿ���Ч�������Чֱ����ֹ����
        delete _it->second;																			// ɾ�����Ӷ���
        mConnMap.erase(clientFd);																	// ������ӳ������Ƴ�����
		remove_by_value(device_match, clientFd);													// ���豸ӳ������Ƴ��Ͽ����ӵ��豸
    }

    mDisConnList.clear();																		// ��նϿ������б�



}