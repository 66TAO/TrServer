#pragma once

#include <mysql.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include "MegParse.h"
#include "mail.h"
#include "../Base/Log.h"
#define MYSQL_IP  "127.0.0.1"    	   	   // 数据库IP
#define MYSQL_PWD "Tr708708708"            // 数据库密码(阿里云服务器)
//#define MYSQL_PWD "f166ad5626"            // 数据库密码(本地服务器)
#define MYSQL_USER "Ship_DB" 			   // 数据库账号
#define MYSQL_NAME "ship_db"      		   // 数据库名称(阿里云服务器)
//#define MYSQL_NAME "Ship_DB"      		   // 数据库名称(本地服务器)

class TrDatabase
{
public:
	static TrDatabase* createNew();
	TrDatabase();
	bool init_db();
	void handle_04(struct isr_mess*& isr_mess_reg);
	void handle_05(struct sap_mess*& sap_mess_reg);
	void handle_06(struct SAP_DATA*& sap_data);
	void handle_33(struct sap_data_33*& sap_data);
	void ship_9001_isport(std::string Wd, std::string Jd, const char* time);
	void ship_6001_isport(std::string Wd, std::string Jd, const char* time);
	void sendmail(std::string Message);
	void read_wheather(struct SAP_DATA*& sap_data);

private:
	void insert_total(struct isr_mess*& p);
	void update_total(struct isr_mess*& p);
	void insert_isr(struct sap_mess*& p);
	void update_isr(struct sap_mess*& p);
	void insert_sap(struct SAP_DATA*& q);

private:
	int state;
	char st_query[4096] = {};
	int flag = 0;
	std::string temp_sql;
	MYSQL db_g2020;
	MYSQL* connection;
	MYSQL_RES* res;
	MYSQL_ROW row;

};