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
    UsageEnvironment* env() const {//const��������Ա�������������޸���ĳ�Ա������ֵ
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
    std::map<int, TrConnection*> mConnMap; // <clientFd,conn> ά�����б�����������
    std::vector<int> mDisConnList;//���б�ȡ�������� clientFd
    TriggerEvent* mCloseTriggerEvent;// �ر����ӵĴ����¼�
    //std::unique_ptr<IOEvent> mAcceptIOEvent; // ʹ������ָ�����IOEvent
    //std::unique_ptr<TriggerEvent> mCloseTriggerEvent; // ʹ������ָ�����TriggerEvent

};
