#pragma once

#include "../Scheduler/UsageEnvironment.h"
#include "../Scheduler/Event.h"
#include "../Scheduler/EventScheduler.h"
#include "InetAddress.h"
#include "TrDatabase.h"
#include <map>

class TrConnection;
class TrServer {

public:
    static TrServer* createNew(UsageEnvironment* env, Ipv4Address& addr, TrDatabase* trdb);

    TrServer(UsageEnvironment* env, Ipv4Address& addr, TrDatabase* trdb);
    ~TrServer();

public:
    void start();
    UsageEnvironment* env() const {//const（常量成员函数）：不会修改类的成员变量的值
        return mEnv;
    }
private:
    static void readCallback(void*);
    void handleRead();
    static void cbDisConnect(void* arg, int clientFd);
    void handleDisConnect(int clientFd);
    static void cbCloseConnect(void* arg);
    void handleCloseConnect();
    int cbAcceptConnect();

private:
    void thread_process_send_message();
    char* protocol_pack_mach_open();
    char* protocol_pack_mach_close();
    void remove_by_value(std::map<std::string, int>& map, const int& value);
    UsageEnvironment* mEnv;
    int mFd;
    Ipv4Address mAddr;
    bool mListen;
    IOEvent* mAcceptIOEvent;
    TrDatabase* Trdb;
    std::map<int, TrConnection*> mConnMap; // <clientFd,conn> 维护所有被创建的连接
    std::vector<int> mDisConnList;//所有被取消的连接 clientFd
    TriggerEvent* mCloseTriggerEvent;// 关闭连接的触发事件
    //std::unique_ptr<IOEvent> mAcceptIOEvent; // 使用智能指针管理IOEvent
    //std::unique_ptr<TriggerEvent> mCloseTriggerEvent; // 使用智能指针管理TriggerEvent

};
