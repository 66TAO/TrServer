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

    mFd = sockets::createTcpSock();															//非阻塞的描述符
    sockets::setReuseAddr(mFd, 1);															//设置套接字选项（重用端口）（重启时可能端口并未释放，加上后可以及时重用端口）
    if (!sockets::bind(mFd, addr.getIp(), mAddr.getPort())) {
        return;
    }

    LOGI("rtsp://%s:%d fd=%d", addr.getIp().data(), addr.getPort(), mFd);					//服务端的ip、端口当前套接字

    mAcceptIOEvent = IOEvent::createNew(mFd, this);
    mAcceptIOEvent->setReadCallback(readCallback);											//←主要数据处理//设置回调的socket可读 函数指针端口建立连接事件
    mAcceptIOEvent->enableReadHandling();													//设置为可读

    mCloseTriggerEvent = TriggerEvent::createNew(this);
    mCloseTriggerEvent->setTriggerCallback(cbCloseConnect);									//设置回调的关闭连接 函数指针

}

TrServer::~TrServer()
{
    if (mListen)
        mEnv->scheduler()->removeIOEvent(mAcceptIOEvent);

    delete mAcceptIOEvent;
    delete mCloseTriggerEvent;

    sockets::close(mFd);
}

char* TrServer::protocol_pack_mach_open()												// 创建并返回"open"协议包字符串
{
	char* res_pack = (char*)malloc(10);													//malloc：分配 10 个字节的内存空间，并将其地址赋给字符指针变量 res_pack(可加判断其是否创建成功)
	if (res_pack == nullptr) {
		cerr << "Memory allocation failed!" << endl;
		exit(EXIT_FAILURE);
	}
	strcpy(res_pack, "open");															//将 "open" 复制到了 res_pack 所指向的内存空间中
	return res_pack; 
}

char* TrServer::protocol_pack_mach_close()												// 创建并返回"close"协议包字符串
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
	char* TS_buf = protocol_pack_mach_open();												// 获取"open"协议包字符串
	char* TS_buf1 = protocol_pack_mach_close();												// 获取"close"协议包字符串
	string key1 = "ISR0400000000004";
	string key2 = "ISR0112912310101";
	string key3 = "ISR1500000000015";
	string key4 = "ISR1100000000011";
	while (1) {
		while (cin >> b) {
			switch (b) {
			case '1':
				if (device_match.count(key1) > 0) {
					send(device_match[key1], TS_buf, strlen(TS_buf), MSG_DONTWAIT);//mes_fd套接字文件描述符，strlen(TS_buf)指明发送的字节数长度，MSG_DONTWAIT非阻塞模式发送
					cout << "Reply 04 open successfully!!" << endl;
				}
				else {
					cout << "ISR04 be not online" << endl;
				}
				break;
			case '2':
				if (device_match.count(key1) > 0) {
					send(device_match[key1], TS_buf1, strlen(TS_buf1), MSG_DONTWAIT);//mes_fd套接字文件描述符，strlen(TS_buf)指明发送的字节数长度，MSG_DONTWAIT非阻塞模式发送
					cout << "Reply 04 close successfully!!" << endl;
				}
				else {
					cout << "ISR04 be not online" << endl;
				}
				break;
			case '3':
				if (device_match.count(key2) > 0) {
					send(device_match[key2], TS_buf, strlen(TS_buf), MSG_DONTWAIT);//mes_fd套接字文件描述符，strlen(TS_buf)指明发送的字节数长度，MSG_DONTWAIT非阻塞模式发送
					cout << "Reply 01 message successfully!!" << endl;
				}
				else {
					cout << "ISR01 be not online" << endl;
				}
				break;
			case '4':
				if (device_match.count(key2) > 0) {
					send(device_match[key2], TS_buf1, strlen(TS_buf1), MSG_DONTWAIT);//mes_fd套接字文件描述符，strlen(TS_buf)指明发送的字节数长度，MSG_DONTWAIT非阻塞模式发送
					cout << "Reply 01 close successfully!!" << endl;
				}
				else {
					cout << "ISR01 be not online" << endl;
				}
				break;
			case '5':
				if (device_match.count(key3) > 0) {
					send(device_match[key3], TS_buf, strlen(TS_buf), MSG_DONTWAIT);//mes_fd套接字文件描述符，strlen(TS_buf)指明发送的字节数长度，MSG_DONTWAIT非阻塞模式发送
					cout << "Reply 15 message successfully!!" << endl;
				}
				else {
					cout << "ISR15 be not online" << endl;
				}
				break;
			case '6':
				if (device_match.count(key3) > 0) {
					send(device_match[key3], TS_buf1, strlen(TS_buf1), MSG_DONTWAIT);//mes_fd套接字文件描述符，strlen(TS_buf)指明发送的字节数长度，MSG_DONTWAIT非阻塞模式发送
					cout << "Reply 15 close successfully!!" << endl;
				}
				else {
					cout << "ISR15 be not online" << endl;
				}
				break;
			case '7':
				if (device_match.count(key4) > 0) {
					send(device_match[key4], TS_buf, strlen(TS_buf), MSG_DONTWAIT);//mes_fd套接字文件描述符，strlen(TS_buf)指明发送的字节数长度，MSG_DONTWAIT非阻塞模式发送
					cout << "Reply 11 message successfully!!" << endl;
				}
				else {
					cout << "ISR11 be not online" << endl;
				}
				break;
			case '8':
				if (device_match.count(key4) > 0) {
					send(device_match[key4], TS_buf1, strlen(TS_buf1), MSG_DONTWAIT);//mes_fd套接字文件描述符，strlen(TS_buf)指明发送的字节数长度，MSG_DONTWAIT非阻塞模式发送
					cout << "Reply 11 close successfully!!" << endl;
				}
				else {
					cout << "ISR11 be not online" << endl;
				}
				break;
			case '9':
				for (auto b : device_match) {
					cout << b.first << " " << b.second << endl;							// 打印设备映射关系
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

void TrServer::remove_by_value(std::map<std::string, int>& map, const int& value)				// 移除map中特定的值
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

void TrServer::start() {															// 启动服务器
    LOGI("Start thread");												
	std::thread t1(&TrServer::thread_process_send_message, this);		// 创建一个线程执行消息发送任务
	t1.detach();														// 分离线程，使其在后台运行
    mListen = true;														// 标记服务器监听状态为true
    sockets::listen(mFd, 60);											// 开始监听客户端连接请求，backlog设置为60（最大链接个数）
    mEnv->scheduler()->addIOEvent(mAcceptIOEvent);						// 将Accept事件添加到事件调度器中
    Trdb->init_db();													// 初始化数据库
}

void TrServer::readCallback(void* arg) {								// 进行读操作的回调函数
    TrServer* trServer = (TrServer*)arg;
    trServer->handleRead();												//←主要的数据处理
		
}

void TrServer::handleRead() {											//处理mAcceptIOEvent// 处理读事件
    int clientFd = sockets::accept(mFd);								// 接受客户端连接，获取客户端套接字文件描述符
    if (clientFd < 0)
    {
        LOGE("handleRead error,clientFd=%d", clientFd);
        return;
    }
    TrConnection* conn = TrConnection::createNew(this, Trdb, clientFd);		// 创建新的连接对象    ←主要的数据处理
    conn->setDisConnectCallback(TrServer::cbDisConnect, this);				// 设置连接断开回调函数
    mConnMap.insert(std::make_pair(clientFd, conn));						// 将连接对象插入连接映射表中

}
void TrServer::cbDisConnect(void* arg, int clientFd) {					// 连接断开的回调函数
    TrServer* server = (TrServer*)arg;

    server->handleDisConnect(clientFd);
}

void TrServer::handleDisConnect(int clientFd) {								// 处理连接断开事件

	remove_by_value(device_match, clientFd);							// 从设备映射表中移除断开连接的设备

    mDisConnList.push_back(clientFd);									// 将断开连接的客户端套接字文件描述符添加到断开连接列表中

    mEnv->scheduler()->addTriggerEvent(mCloseTriggerEvent);				// 添加关闭连接触发事件到事件调度器中

}

void TrServer::cbCloseConnect(void* arg) {							// 关闭连接的回调函数
    TrServer* server = (TrServer*)arg;
    server->handleCloseConnect();
}
void TrServer::handleCloseConnect() {																// 处理关闭连接事件


    for (std::vector<int>::iterator it = mDisConnList.begin(); it != mDisConnList.end(); ++it) {

        int clientFd = *it;									//it指向的键值对中的键赋值给clientFd
        std::map<int, TrConnection*>::iterator _it = mConnMap.find(clientFd);
        assert(_it != mConnMap.end());						//进行断言检查检查指向是否有效，如果无效直接终止程序
        delete _it->second;																			// 删除连接对象
        mConnMap.erase(clientFd);																	// 从连接映射表中移除连接
		remove_by_value(device_match, clientFd);													// 从设备映射表中移除断开连接的设备
    }

    mDisConnList.clear();																		// 清空断开连接列表



}