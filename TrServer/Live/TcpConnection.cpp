#include "TcpConnection.h"
#include "../Scheduler/SocketsOps.h"
#include "../Base/Log.h"
#include "../Scheduler/EventScheduler.h"


TcpConnection::TcpConnection(UsageEnvironment* env, int clientFd) :
    mEnv(env),
    mClientFd(clientFd)
{
    mClientIOEvent = IOEvent::createNew(clientFd, this);                    // ����һ���µ� IOEvent ����������ͻ����ļ��������� I/O �¼�
    mClientIOEvent->setReadCallback(readCallback);                          //����Ҫ����database// ���ö���д�ʹ���Ļص�����
    mClientIOEvent->setWriteCallback(writeCallback);
    mClientIOEvent->setErrorCallback(errorCallback);
    mClientIOEvent->enableReadHandling(); //Ĭ��ֻ������

    mClienttimeEvent = TimerEvent::createNew(this);
    mClienttimeEvent->setTimeoutCallback(timeOutdisconnectCallback);
    //mClienttimeEvent->start();
    mtimeid = mEnv->scheduler()->addTimerEventRunEvery(mClienttimeEvent, 15 * 60 * 1000);//����22���Ķ�ʱ���Ϊ10����10 * 60 * 1000


    //    mClientIOEvent->enableWriteHandling();
    //    mClientIOEvent->enableErrorHandling();

    mEnv->scheduler()->addIOEvent(mClientIOEvent);//����ͻ���IO�¼�           // ���ͻ��� IOEvent ��ӵ��������У��Ա�����¼�����
}

TcpConnection::~TcpConnection()
{
    mEnv->scheduler()->removeIOEvent(mClientIOEvent);                   // �ӵ��������Ƴ��ͻ��� IOEvent
    mEnv->scheduler()->removeTimerEventRunEvery(mtimeid);
    delete mClientIOEvent;                                              // ɾ���ͻ��� IOEvent ����
    delete mClienttimeEvent;
    //    Delete::release(mClientIOEvent);

    sockets::close(mClientFd);                                          // �رտͻ����ļ�������
}

void TcpConnection::setDisConnectCallback(DisConnectCallback cb, void* arg)         // ���öϿ����ӻص������������
{
    mDisConnectCallback = cb;
    mArg = arg;
}

void TcpConnection::enableReadHandling()                                // ���ö�ȡ����
{
    if (mClientIOEvent->isReadHandling())                               // ����Ѿ������˶�ȡ������ֱ�ӷ���
        return;

    mClientIOEvent->enableReadHandling();                               // ������ȡ����
    mEnv->scheduler()->updateIOEvent(mClientIOEvent);                   // ���¿ͻ��� IOEvent ���¼�״̬����������
}

void TcpConnection::enableWriteHandling()
{
    if (mClientIOEvent->isWriteHandling())
        return;

    mClientIOEvent->enableWriteHandling();
    mEnv->scheduler()->updateIOEvent(mClientIOEvent);
}

void TcpConnection::enableErrorHandling()
{
    if (mClientIOEvent->isErrorHandling())
        return;

    mClientIOEvent->enableErrorHandling();
    mEnv->scheduler()->updateIOEvent(mClientIOEvent);
}

void TcpConnection::disableReadeHandling()                          // ���ö�ȡ����
{
    if (!mClientIOEvent->isReadHandling())                          // ���δ������ȡ������ֱ�ӷ���
        return;

    mClientIOEvent->disableReadeHandling();                         // ���ö�ȡ����
    mEnv->scheduler()->updateIOEvent(mClientIOEvent);               // ���¿ͻ��� IOEvent ���¼�״̬����������
}

void TcpConnection::disableWriteHandling()
{
    if (!mClientIOEvent->isWriteHandling())
        return;

    mClientIOEvent->disableWriteHandling();
    mEnv->scheduler()->updateIOEvent(mClientIOEvent);
}

void TcpConnection::disableErrorHandling()
{
    if (!mClientIOEvent->isErrorHandling())
        return;

    mClientIOEvent->disableErrorHandling();
    mEnv->scheduler()->updateIOEvent(mClientIOEvent);
}

void TcpConnection::handleRead() {                                  // �����ȡ�¼�

    //LOGI("");
    int ret = mInputBuffer.read(mClientFd);                         // �ӿͻ��˶�ȡ���ݵ����뻺����

    if (ret <= 0)                                                   // �����ȡ������߶�ȡ���ֽ���С�ڵ��� 0������Ϊ�ͻ����Ѿ��Ͽ�����
    {
        for (const auto& pair : device_match) {
            if (pair.second == mClientFd) {
                string ISR_id = pair.first;
                LOGE("read error,fd=%d,ret=%d,ISRid=%s", mClientFd, ret, ISR_id.c_str());
                handleDisConnect();                                         // ����Ͽ������¼�
                return;
            }
        }
        LOGE("read error,fd=%d,ret=%d,no ISRid", mClientFd, ret);
        handleDisConnect();                                         // ����Ͽ������¼�
        return;
    }

    /* ��ȡ���� */
    //this->disableReadeHandling();

    handleReadBytes();// ����RtspConnecton�����ʵ�ֺ���             // �����ȡ��������         ����Ҫ�������ݺ���
}

void TcpConnection::handleReadBytes() {//TrConnection��д�÷���       ������д
    LOGI("");
}
void TcpConnection::handleDisConnect()
{
    if (mDisConnectCallback) {                                      // ��������˶Ͽ����ӻص�����������øûص�����
        mDisConnectCallback(mArg, mClientFd);
    }
}
void TcpConnection::handleWrite()
{
    LOGI("");

}

void TcpConnection::handleError()
{
    LOGI("");
}

void TcpConnection::readCallback(void* arg)                         //���Ļص�����
{
    TcpConnection* tcpConnection = (TcpConnection*)arg;
    tcpConnection->handleRead();                                        //����Ҫ���ݴ���
}

void TcpConnection::writeCallback(void* arg)
{
    TcpConnection* tcpConnection = (TcpConnection*)arg;
    tcpConnection->handleWrite();
}

void TcpConnection::errorCallback(void* arg)
{
    TcpConnection* tcpConnection = (TcpConnection*)arg;
    tcpConnection->handleError();
}

void TcpConnection::timeOutdisconnectCallback(void* arg) {
    TcpConnection* timeOutdisconnect = (TcpConnection*)arg;
    for (const auto& pair : device_match) {
        if (pair.second == timeOutdisconnect->mClientFd) {
            string ISR_id = pair.first;
            LOGE("time out, will close the fd=%d,timeid=%d,ISRid=%s", timeOutdisconnect->mClientFd, timeOutdisconnect->mtimeid, ISR_id.c_str());
            timeOutdisconnect->handleDisConnect();                                         // ����Ͽ������¼�
            return;
        }
    }
    LOGI("time out, will close the fd=%d,timeid=%d, no ISRid", timeOutdisconnect->mClientFd, timeOutdisconnect->mtimeid);
    //cout << "time out will close the fd:" << timeOutdisconnect->mClientFd << endl;
    //cout << "time out will close the timefd:" << timeOutdisconnect->mtimeid << endl;
    timeOutdisconnect->handleDisConnect();
}

