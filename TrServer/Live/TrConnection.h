#pragma once

#include <map>
#include "TcpConnection.h"
#include <time.h>
#include <algorithm> 
#include <math.h>
#include <random>
#include "MegParse.h"
#include "TrDatabase.h"



class TrServer;
class TrDatabase;
class TrConnection : public TcpConnection                   //TrConnection ��̳��� TcpConnection ��
{
public:
    static TrConnection* createNew(TrServer* TrServer, TrDatabase* TrDatabase, int clientFd);

    TrConnection(TrServer* TrServer, TrDatabase* TrDatabase, int clientFd);
    virtual ~TrConnection();                                //virtual�麯��������������


protected:
    virtual void handleReadBytes();

private:
    void parse_04(const std::string& meg);
    void parse_05(const std::string& meg);
    void parse_06(const std::string& meg);
    void parse_22(const std::string& meg);
    void parse_30();
    void parse_31();
    void parse_33(const std::string& meg);

    char* getTime();
    //����ת��
    int HextoDec(const char HexNum[]);
    //ת���洢ʱ��
    std::string trans_time(std::string& obj);
    //��װ10��
    char* protocol_pack_10();
    double generateRandomDouble(double min, double max);
    void SAP_DATA_GET(const string& obj, int falg_lenth, const std::string Rtd, double& DATA);
    void Equipment_DATA_GET(const string& obj, SAP_DATA*& p);
    void set_DATA_water(struct SAP_DATA*& p);
    void set_DATA_air(struct SAP_DATA*& p);
    void set_DATA_3001(struct SAP_DATA*& p);
    void set_DATA_6001(struct SAP_DATA*& p);
    void set_DATA_car(struct SAP_DATA*& p);
    void parse_DATA(const string& obj, struct SAP_DATA*& p);
    void parse_water(const std::string& obj, struct SAP_DATA*& p);
    void parse_air(const std::string& obj, struct SAP_DATA*& p);
    void parse_3001_ship(const string& obj, struct SAP_DATA*& p);
    void parse_car(const string& obj, struct SAP_DATA*& p);
    void parse_6001_ship(const string& obj, struct SAP_DATA*& p);

private:
    std::string mPeerIp;
    TrDatabase* Trdb;
    int clientfd_cur;
    char tmp_time[64];//ʱ�������
    void* mArg;
};
