#include "TrConnection.h"
#include "TrServer.h"
#include <stdio.h>
#include "../Base/Log.h"
#include <string.h>
#include <iostream>

std::map<std::string, int> device_match;			//第一个是键，第二个是值

static void getPeerIp(int fd, std::string& ip)
{
	struct sockaddr_in addr;														// 创建一个 IPv4 地址结构体
	socklen_t addrlen = sizeof(struct sockaddr_in);									// 获取地址结构体的大小
	getpeername(fd, (struct sockaddr*)&addr, &addrlen);								// 使用 getpeername 函数获取与指定文件描述符相关联的对等端地址信息
	ip = inet_ntoa(addr.sin_addr);													// 将对等端的 IP 地址转换为字符串，并存储在传入的 std::string 对象 ip 中
}

TrConnection* TrConnection::createNew(TrServer* TrServer, TrDatabase* TrDatabase, int clientFd)				// 创建一个新的 TrConnection 对象并返回指针
{
	return new TrConnection(TrServer, TrDatabase, clientFd);
}

TrConnection::TrConnection(TrServer* TrServer, TrDatabase* TrDatabase, int clientFd) :
	TcpConnection(TrServer->env(), clientFd),											// 调用 TcpConnection 的构造函数，传入 TrServer 的环境变量指针和客户端文件描述符
	Trdb(TrDatabase),																	// 初始化 Trdb 成员变量为传入的 TrDatabase 指针
	clientfd_cur(clientFd)																// 初始化 clientfd_cur 成员变量为传入的客户端文件描述符
{
	LOGI("TrConnection() mClientFd=%d", mClientFd);										// 输出日志，显示客户端文件描述符
	getPeerIp(clientFd, mPeerIp);														// 获取与客户端连接的对等端 IP 地址，并存储到 mPeerIp 成员变量中

}

TrConnection::~TrConnection()
{
	LOGI("~TrConnection() mClientFd=%d", mClientFd);
}

void TrConnection::handleReadBytes()
{
	std::cout << mInputBuffer.getmessage() << std::endl;
	string meg = mInputBuffer.getmessage();							// 从输入缓冲区中获取完整的消息
	string protocol = meg.substr(pct_st, pct_len);					// 提取消息中的协议字段

	if (protocol == ISR_REGIST)						// 根据协议字段分发消息处理函数
	{
		parse_04(meg);				//插入isr相关数据
	}
	else if (protocol == AP_REGIST)
	{
		parse_05(meg);				//注册or升级更新sap
	}
	else if (protocol == SAP_TRANS_DATA)
	{
		parse_06(meg);				//主要水质、空气分析 
	}
	else if (protocol == ISR_UPD_REGIST)
	{
		parse_22(meg);				//找到当前SIR设备和文件描述符fd(File descriptor)
	}
	else if (protocol == ISR_REPLY1)
	{
		parse_30();
	}
	else if (protocol == ISR_REPLY2)
	{
		parse_31();
	}
	else if (protocol == ISR_CHANGE)
	{
		parse_33(meg);				//测试isr、sap接受数据
	}
	else
	{
		cout << "The protocol is not used and the data sent is : " << meg << endl;
	}
}

char* TrConnection::getTime()							//获取时间 //当地时间应该是本机电脑的时间
{
	time_t timep;																	// 声明一个 time_t 类型的变量，用于存储时间信息
	time(&timep);																	// 获取当前时间，将其保存在 timep 变量中
	strftime(tmp_time, sizeof(tmp_time), "%Y-%m-%d %H:%M:%S", localtime(&timep));	// 使用 strftime 函数将时间格式化为字符串，并将结果存储在 tmp_time 数组中
	return tmp_time;
}

int TrConnection::HextoDec(const char HexNum[])					//16进制转10进制
{
	int tempHex = 0, sumDec = 0;
	int n = strlen(HexNum);
	for (int i = 0; HexNum[i] != '\0'; i++)
	{
		switch (HexNum[i])
		{
		case 'A': tempHex = 10; break;
		case 'B': tempHex = 11; break;
		case 'C': tempHex = 12; break;
		case 'D': tempHex = 13; break;
		case 'E': tempHex = 14; break;
		case 'F': tempHex = 15; break;
		default: tempHex = HexNum[i] - '0';
		}
		sumDec += tempHex * pow(16, n - 1 - i);
	}
	return sumDec;
}

string TrConnection::trans_time(string& obj)
{
	//2021 08 04 11 22 32 983
	//2021-08-27 14:32:54
	string res_time, temp_time;

	temp_time = obj.substr(0, 4);				//从第0位开始4个字符提取出来   即2021（年）
	temp_time += "-";
	res_time += temp_time;
	temp_time = obj.substr(4, 2);				//从第4位开始2个字符   即08月（月）
	temp_time += "-";
	res_time += temp_time;
	temp_time = obj.substr(6, 2);				//从第6位开始2个字符  即27（日）
	temp_time += " ";
	res_time += temp_time;

	temp_time = obj.substr(8, 2);				//从第8位开始2个字符  即14（时）
	temp_time += ":";
	res_time += temp_time;

	temp_time = obj.substr(10, 2);				//从第10位开始2个字符  即32（分）
	temp_time += ":";
	res_time += temp_time;
	temp_time = obj.substr(12, 2);				//从第12位开始2个字符  即54（秒）
	res_time += temp_time;
	return res_time;
}

double TrConnection::generateRandomDouble(double min, double max)				//在min—max之间生成一个随机的双精度浮点数
{
	random_device rd;												//生成随机数种子
	mt19937 gen(rd());												// 创建梅森旋转发生器，并用随机设备生成的种子进行初始化
	uniform_real_distribution<double> dis(min, max);				// 创建一个均匀分布的双精度浮点数对象，范围在 min 到 max 之间
	return dis(gen);												// 利用分布对象和梅森旋转发生器生成并返回一个随机双精度浮点数
}

char* TrConnection::protocol_pack_10()
{
	static char res_pack[40];										// 静态数组，用于存储结果数据包，大小为40字节
	char* timedata = getTime();										// 获取时间数据的函数

	sprintf(res_pack, "$10FFFFFFF00%d%s@", sizeof(timedata), timedata);			// 将timedata的内容格式化后写入res_pack字符串中
	return res_pack;
}

void TrConnection::SAP_DATA_GET(const string& obj, int falg_lenth, const std::string Rtd, double& DATA)
{
	int flag_start;
	string sensor_temp;
	flag_start = obj.find(Rtd) + sensor_flag;					//起始字段
	if (flag_start < 11)											//数据获取错误
	{
		DATA = -999.99;
	}
	else {
		sensor_temp = obj.substr(flag_start, falg_lenth);
		DATA = atof(sensor_temp.c_str());
		if (DATA >= 999999.999)
		{
			DATA = -999.99;
		}
	}
	cout << Rtd << ":" << DATA;
	sensor_temp.clear();
}

void TrConnection::Equipment_DATA_GET(const string& obj, SAP_DATA*& p)
{
	int flag_start;
	string sensor_temp;

	cout << "<<<<Current time>>>>" << endl;
	//获取系统时间的函数。
	strcpy(p->air_real_time, getTime());
	cout << "air_real_time:" << p->air_real_time;

	cout << "<<<<Equipment data>>>>" << endl;
	flag_start = obj.find(TIME_DEV) + time_mac_flag;
	sensor_temp = obj.substr(flag_start, time_lenth);
	strcpy(p->air_com_time, trans_time(sensor_temp).c_str());
	cout << "air_com_time:" << p->air_com_time;

	flag_start = obj.find(MAC_DEV) + time_mac_flag;
	strcpy(p->air_mac, obj.substr(flag_start, mac_lenth).c_str());
	strcpy(p->air_id, obj.substr(flag_start + 3, 2).c_str());
	cout << " air_mac:" << p->air_mac << " air_id:" << p->air_id;

	flag_start = obj.find(GPS_DEV) + gps_flag;
	strcpy(p->sap_gps, obj.substr(flag_start, gps_lenth).c_str());
	cout << " sap_dev_gps:" << p->sap_gps;

	strcpy(p->data_type, "100");
	sensor_temp.clear();
}

void TrConnection::set_DATA_6001(struct SAP_DATA*& p)
{
	//p->so2 = -999.99;
	p->h2s = -999.99;
	p->nh3 = -999.99;
	//p->no2 = -999.99;
	p->no = -999.99;
	//p->co = -999.99;
	//p->co2 = -999.99;
	p->o2 = -999.99;
	//p->o3 = -999.99;
	p->ch4 = -999.99;
	p->pm10 = -999.99;
	p->pmD4 = -999.99;
	p->pm1 = -999.99;
	//p->hum = -999.99;
	//p->WATER_TEMPER = -999.99;
	//p->ph = -999.99;
	//p->conductivity = -999.99;
	//p->turbidity = -999.99;
	//p->dissolved_ox = -999.99;
	//p->cod = -999.99;
	p->toc = -999.99;
	//p->nh3n = -999.99;
	//p->temper = -999.99;
	//p->bg_algae = -999.99;
	p->cupric_ion = -999.99;
	p->cadmium_ion = -999.99;
	p->depth_water = -999.99;
	p->wind_speed = -999.99;
	p->wind_direction = -999.99;
	p->wind_speed_10m = -999.99;
	p->ambient_temp = -999.99;
	p->max_temp = -999.99;
	p->min_temp = -999.99;
	p->ambient_humi = -999.99;
	p->dewp_humi = -999.99;
	p->air_press = -999.99;
	//p->luminous = -999.99;
	p->rain_fall = -999.99;
}

void TrConnection::set_DATA_car(struct SAP_DATA*& p)
{
	//p->so2 = -999.99;
	p->h2s = -999.99;
	p->nh3 = -999.99;
	//p->no2 = -999.99;
	p->no = -999.99;
	//p->co = -999.99;
	//p->co2 = -999.99;
	p->o2 = -999.99;
	//p->o3 = -999.99;
	p->ch4 = -999.99;
	p->pm10 = -999.99;
	p->pmD4 = -999.99;
	p->pm1 = -999.99;
	//p->hum = -999.99;
	p->WATER_TEMPER = -999.99;
	p->ph = -999.99;
	p->conductivity = -999.99;
	p->turbidity = -999.99;
	p->dissolved_ox = -999.99;
	p->cod = -999.99;
	p->toc = -999.99;
	p->nh3n = -999.99;
	//p->temper = -999.99;
	p->bg_algae = -999.99;
	p->cupric_ion = -999.99;
	p->cadmium_ion = -999.99;
	p->depth_water = -999.99;
	p->wind_speed = -999.99;
	p->wind_direction = -999.99;
	p->wind_speed_10m = -999.99;
	p->ambient_temp = -999.99;
	p->max_temp = -999.99;
	p->min_temp = -999.99;
	p->ambient_humi = -999.99;
	p->dewp_humi = -999.99;
	p->air_press = -999.99;
	//p->luminous = -999.99;
	p->rain_fall = -999.99;
}

void TrConnection::set_DATA_3001(struct SAP_DATA*& p)
{
	/*if (p->so2 == 0) p->so2 = -999.99;
	if (p->h2s == 0) p->h2s = -999.99;
	if (p->nh3 == 0) p->nh3 = -999.99;
	if (p->no2 == 0) p->no2 = -999.99;
	if (p->no == 0) p->no = -999.99;
	if (p->co == 0) p->co = -999.99;
	if (p->co2 == 0) p->co2 = -999.99;
	if (p->o2 == 0) p->o2 = -999.99;
	if (p->o3 == 0) p->o3 = -999.99;
	if (p->ch4 == 0) p->ch4 = -999.99;
	if (p->pm10 == 0) p->pm10 = -999.99;
	if (p->pmD4 == 0) p->pmD4 = -999.99;
	if (p->temper == 0) p->temper = -999.99;
	if (p->hum == 0) p->hum = -999.99;
	if (p->pm1 == 0) p->pm1 = -999.99;
	if (p->WATER_TEMPER == 0) p->WATER_TEMPER = -999.99;
	if (p->ph == 0) p->ph = -999.99;
	if (p->conductivity == 0) p->conductivity = -999.99;
	if (p->turbidity == 0) p->turbidity = -999.99;
	if (p->dissolved_ox == 0) p->dissolved_ox = -999.99;
	if (p->cod == 0) p->cod = -999.99;
	if (p->toc == 0) p->toc = -999.99;
	if (p->nh3n == 0) p->nh3n = -999.99;
	if (p->chlorophyll == 0) p->chlorophyll = -999.99;
	if (p->bg_algae == 0) p->bg_algae = -999.99;
	if (p->cupric_ion == 0) p->cupric_ion = -999.99;
	if (p->cadmium_ion == 0) p->cadmium_ion = -999.99;
	if (p->flow_velocity == 0) p->flow_velocity = -999.99;
	if (p->depth_water == 0) p->depth_water = -999.99;
	if (p->wind_direction == 0) p->wind_direction = -999.99;
	if (p->wind_speed == 0) p->wind_speed = -999.99;
	if (p->wind_speed_10m == 0) p->wind_speed_10m = -999.99;
	if (p->ambient_temp == 0) p->ambient_temp = -999.99;
	if (p->max_temp == 0) p->max_temp = -999.99;
	if (p->min_temp == 0) p->min_temp = -999.99;
	if (p->ambient_humi == 0) p->ambient_humi = -999.99;
	if (p->dewp_humi == 0) p->dewp_humi = -999.99;
	if (p->air_press == 0) p->air_press = -999.99;
	if (p->luminous == 0) p->luminous = -999.99;
	if (p->rain_fall == 0) p->rain_fall = -999.99;*/

	p->so2 = -999.99;
	p->h2s = -999.99;
	p->nh3 = -999.99;
	p->no2 = -999.99;
	p->no = -999.99;
	p->co = -999.99;
	p->co2 = -999.99;
	p->o2 = -999.99;
	p->o3 = -999.99;
	p->ch4 = -999.99;
	p->pm10 = -999.99;
	p->pmD4 = -999.99;
	p->pm1 = -999.99;
	p->hum = -999.99;
	//p->WATER_TEMPER = -999.99;
	//p->ph = -999.99;
	//p->conductivity = -999.99;
	p->turbidity = -999.99;
	//p->dissolved_ox = -999.99;
	p->cod = -999.99;
	//p->toc = -999.99;
	//p->nh3n = -999.99;
	p->temper = -999.99;
	//p->bg_algae = -999.99;
	//p->cupric_ion = -999.99;
	//p->cadmium_ion = -999.99;
	p->depth_water = -999.99;
	p->wind_speed = -999.99;
	p->wind_direction = -999.99;
	p->wind_speed_10m = -999.99;
	p->ambient_temp = -999.99;
	p->max_temp = -999.99;
	p->min_temp = -999.99;
	p->ambient_humi = -999.99;
	p->dewp_humi = -999.99;
	p->air_press = -999.99;
	p->luminous = -999.99;
	p->rain_fall = -999.99;

	//p->so2 = -999.99;
	//p->h2s = -999.99;
	//p->nh3 = -999.99;
	//p->no2 = -999.99;
	//p->no = -999.99;
	//p->co = -999.99;
	//p->co2 = -999.99;
	//p->o2 = -999.99;
	//p->o3 = -999.99;
	//p->ch4 = -999.99;
	//p->ethanol1 = -999.99;
	//p->ethanol2 = -999.99;
	//p->ethanol3 = -999.99;
	//p->pm10 = -999.99;
	//p->pmD4 = -999.99;
	//p->temper = -999.99;
	//p->hum = -999.99;
	//p->pm1 = -999.99;
	////p->WATER_TEMPER = -999.99;
	////p->ph = -999.99;
	////p->conductivity = -999.99;
	//p->turbidity = -999.99;
	////p->dissolved_ox = -999.99;
	//p->cod = -999.99;
	////p->toc = -999.99;
	////p->nh3n = -999.99;
	//p->chlorophyll = -999.99;
	////p->bg_algae = -999.99;
	////p->cupric_ion = -999.99;
	////p->cadmium_ion = -999.99;
	//p->flow_velocity = -999.99;
	//p->depth_water = -999.99;
	//p->wind_direction = -999.99;
	//p->wind_speed = -999.99;
	//p->wind_speed_2m = -999.99;
	//p->wind_speed_10m = -999.99;
	//p->ambient_temp = -999.99;
	//p->max_temp = -999.99;
	//p->min_temp = -999.99;
	//p->ambient_humi = -999.99;
	//p->dewp_humi = -999.99;
	//p->air_press = -999.99;
	//p->luminous = -999.99;
	//p->rain_fall = -999.99;
}

void TrConnection::set_DATA_water(struct SAP_DATA*& p)
{
	p->so2 = -999.99;
	p->h2s = -999.99;
	p->nh3 = -999.99;
	p->no2 = -999.99;
	p->no = -999.99;
	p->co = -999.99;
	//p->co2 = -999.99;
	p->o2 = -999.99;
	p->o3 = -999.99;
	p->ch4 = -999.99;
	p->ethanol1 = -999.99;
	p->ethanol2 = -999.99;
	p->ethanol3 = -999.99;
	//p->pm10 = -999.99;
	//p->pmD4 = -999.99;
	//p->temper = -999.99;
	//p->hum = -999.99;
	//p->wind_speed = -999.99;
	//p->wind_speed_2m = -999.99;
	//p->wind_speed_10m = -999.99;
	p->max_temp = -999.99;
	p->chlorophyll = -999.99;
	//p->toc = -999.99;
	p->ambient_temp = -999.99;
	p->min_temp = -999.99;
	p->ambient_humi = -999.99;
	p->dewp_humi = -999.99;
	//p->air_press = -999.99;
	//p->luminous = -999.99;
	//p->rain_fall = -999.99;
}

void TrConnection::set_DATA_air(struct SAP_DATA*& p)
{
	p->WATER_TEMPER = -999.99;

	p->ph = -999.99;
	p->conductivity = -999.99;
	p->turbidity = -999.99;
	p->dissolved_ox = -999.99;
	p->cod = -999.99;
	p->toc = -999.99;
	p->nh3n = -999.99;
	p->chlorophyll = -999.99;
	p->bg_algae = -999.99;//水质
	p->flow_velocity = -999.99;
	p->depth_water = -999.99;//水文
	p->wind_direction = -999.99;
	p->wind_speed = -999.99;
	p->wind_speed_2m = -999.99;
	p->wind_speed_10m = -999.99;
	p->ambient_temp = -999.99;
	p->max_temp = -999.99;
	p->min_temp = -999.99;
	p->ambient_humi = -999.99;
	p->dewp_humi = -999.99;
	p->air_press = -999.99;
	p->luminous = -999.99;
	p->rain_fall = -999.99;
	//p->depth_water = -999.99;
	//p->flow_velocity = -999.99;
	p->cadmium_ion = -999.99;
	p->cupric_ion = -999.99;
}

void TrConnection::parse_DATA(const string& obj, struct SAP_DATA*& p)
{
	if (strcmp(p->air_isr_id, "04") == 0)
	{
		parse_water(obj, p);
		strcpy(p->ship_data, obj.c_str());//水质数据包
	}
	else if (strcmp(p->air_isr_id, "15") == 0)
	{
		parse_air(obj, p);
		strcpy(p->gas_data, obj.c_str());//南山空气数据包
	}
	else if (strcmp(p->air_isr_id, "20") == 0)
	{
		parse_3001_ship(obj, p);
		strcpy(p->ship_data, obj.c_str());
	}
	else if (strcmp(p->air_isr_id, "21") == 0)
	{
		parse_car(obj, p);
		strcpy(p->gas_data, obj.c_str());
	}
	else if (strcmp(p->air_isr_id, "22") == 0)
	{
		parse_6001_ship(obj, p);
		strcpy(p->ship_data, obj.c_str());
	}
}

void TrConnection::parse_water(const string& obj, struct SAP_DATA*& p)		//分析水质
{
	int flag_start;
	string sensor_temp;
	cout << "<<<<Water quality data>>>>" << endl;

	//SAP_DATA_GET(obj, water_lenth, WATER_TEMPER, p -> WATER_TEMPER);	//水温
	int second_flag_start;

	// 找到第一次出现的位置
	flag_start = obj.find(WATER_TEMPER) + sensor_flag;  // 起始字段
	if (flag_start < 11)  // 数据获取错误
	{
		p->WATER_TEMPER = -999.99;
	}
	else
	{
		// 从第一次出现位置的下一个字符开始，查找第二次出现的位置
		second_flag_start = obj.find(WATER_TEMPER, flag_start + WATER_TEMPER.length()) + sensor_flag;
		if (second_flag_start < 11)  // 第二次查找失败的情况
		{
			p->WATER_TEMPER = -999.99;
		}
		else
		{
			sensor_temp = obj.substr(second_flag_start, water_lenth);
			p->WATER_TEMPER = atof(sensor_temp.c_str());
			if (p->WATER_TEMPER >= 999999.999)
			{
				p->WATER_TEMPER = -999.99;
			}
		}
	}
	cout << "WATER_TEMPER:" << p->WATER_TEMPER;

	SAP_DATA_GET(obj, water_lenth, DISSOLVED_OX, p->dissolved_ox);	//溶解氧
	//SAP_DATA_GET(obj, water_lenth, NH3N_WATER, p -> nh3n);				//氨氮	
	flag_start = obj.find(NH3N_WATER) + sensor_flag;
	if (flag_start < 11)
	{
		p->nh3n = -999.99;
	}
	else {
		sensor_temp = obj.substr(flag_start, water_lenth);
		p->nh3n = atof(sensor_temp.c_str()) / 10.0;
	}
	cout << " nh3n:" << p->nh3n;
	SAP_DATA_GET(obj, water_lenth, CUPRIC_ION, p->cupric_ion);		//铜离子
	//SAP_DATA_GET(obj, water_lenth, CADMIUM_ION, p -> cadmium_ion);		//镉离子
	flag_start = obj.find(CADMIUM_ION) + sensor_flag;
	if (flag_start < 11)
	{
		p->cadmium_ion = -999.99;
	}
	else {
		sensor_temp = obj.substr(flag_start, water_lenth);
		p->cadmium_ion = atof(sensor_temp.c_str()) / 10000.0;
	}
	cout << " cadmium_ion:" << p->cadmium_ion;
	SAP_DATA_GET(obj, water_lenth, BGPI_WATER, p->bg_algae);			//蓝绿藻
	SAP_DATA_GET(obj, water_lenth, PHYII_WATER, p->ph);				//叶绿素
	SAP_DATA_GET(obj, water_lenth, COND_WATER, p->conductivity);		//电导率
	//SAP_DATA_GET(obj, water_lenth, COD, p -> cod);						//化学需氧量
	//SAP_DATA_GET(obj, water_lenth, TURB_WATER, p->turbidity);			//浊度
	SAP_DATA_GET(obj, water_lenth, COD, p->turbidity);			//浊度(解析COD字段，解析的数值是浊度)
	SAP_DATA_GET(obj, water_lenth, TOC_WATER, p->toc);				//总有机碳
	//SAP_DATA_GET(obj, flag_start, sensor_temp, DEPTH_WATER, p -> depth_water);		//深度
	//cout << endl;
	flag_start = obj.find(DEPTH_WATER) + sensor_flag;					//深度
	if (flag_start < 11)
	{
		p->depth_water = -999.99;
	}
	else
	{
		sensor_temp = obj.substr(flag_start, water_lenth);
		p->depth_water = atof(sensor_temp.c_str());

		if (p->depth_water >= 999999.999)
		{
			double minValue = 33.7;    // 最小值
			double maxValue = 68.5;   // 最大值
			p->depth_water = generateRandomDouble(minValue, maxValue);
		}
	}
	cout << " depth_water:" << p->depth_water << endl;


	//大气数据
	cout << "<<<<Atmosphere data>>>>" << endl;

	SAP_DATA_GET(obj, gas_lenth, CO2_SENSOR, p->co2);				//二氧化碳
	SAP_DATA_GET(obj, gas_lenth, PM10_SENSOR, p->pm10);	//PM010
	SAP_DATA_GET(obj, gas_lenth, PM25_SENSOR, p->pmD4);				//PM2.5	
	SAP_DATA_GET(obj, gas_lenth, PM1_SENSOR, p->pm1);		//PM1
	cout << endl;

	//气象数据
	cout << "<<<<Meteorological data>>>>" << endl;

	SAP_DATA_GET(obj, gas_lenth, WIND_DIR, p->wind_direction);			//风向
	SAP_DATA_GET(obj, gas_lenth, WIND_SPE, p->wind_speed);				//风速
	SAP_DATA_GET(obj, gas_lenth, RAIN_FALL, p->rain_fall);				//雨量	
	SAP_DATA_GET(obj, gas_lenth, ILUM, p->luminous);					//光照
	SAP_DATA_GET(obj, gas_lenth, TEMPER_SENSOR, p->temper);				//气温
	SAP_DATA_GET(obj, gas_lenth, HUMID_SENSOR, p->hum);					//湿度
	SAP_DATA_GET(obj, gas_lenth, AIR_PRES, p->air_press);				//气压
	SAP_DATA_GET(obj, gas_lenth, WIND_SPE10M, p->wind_speed_10m);		//噪声
	cout << endl;

	Equipment_DATA_GET(obj, p);

	strcpy(p->gas_data, "notinfo");
	cout << " data_type:" << p->data_type << " gas_data:" << p->gas_data << endl;

	//判断船是否离港
	//string Wd = obj.substr(103, 4);
	//string Jd = obj.substr(114, 4);

	//double Wd1 = atof(Wd.c_str());					//经纬度
	//double Jd1 = atof(Jd.c_str());
	string Wd(p->sap_gps, 9);
	string Jd(p->sap_gps + 10, 10);

	Trdb->ship_isport(Wd, Jd, p->air_real_time);					//判断船是否在岗

	set_DATA_water(p);


	sensor_temp.clear();
	return;
}

void TrConnection::parse_air(const string& obj, struct SAP_DATA*& p)		//分析空气
{
	cout << "<<<<Nan Shan (Yifu Building): atmosphere data>>>>" << endl;

	SAP_DATA_GET(obj, gas_lenth, SO2_SENSOR, p->so2);				//二氧化硫
	SAP_DATA_GET(obj, gas_lenth, O2_SENSOR, p->o2);				//氧气
	SAP_DATA_GET(obj, gas_lenth, O3_SENSOR, p->o3);				//臭氧	
	SAP_DATA_GET(obj, gas_lenth, CH4_SENSOR, p->ch4);				//甲烷
	SAP_DATA_GET(obj, gas_lenth, NO_SENSOR, p->no);				//氧化氮
	SAP_DATA_GET(obj, gas_lenth, NH3_SENSOR, p->nh3);				//氨
	SAP_DATA_GET(obj, gas_lenth, NO2_SENSOR, p->no2);				//二氧化氮
	SAP_DATA_GET(obj, gas_lenth, H2S_SENSOR, p->h2s);				//硫化氢
	SAP_DATA_GET(obj, gas_lenth, CO2_SENSOR, p->co2);				//二氧化碳
	SAP_DATA_GET(obj, gas_lenth, TEMPER_SENSOR, p->temper);		//温度
	SAP_DATA_GET(obj, gas_lenth, HUMID_SENSOR, p->hum);			//湿度
	SAP_DATA_GET(obj, gas_lenth, PM10_SENSOR, p->pm10);			//PM10
	SAP_DATA_GET(obj, gas_lenth, PM25_SENSOR, p->pmD4);			//PM2.5
	SAP_DATA_GET(obj, gas_lenth, CO_SENSOR, p->co);				//一氧化氮
	SAP_DATA_GET(obj, gas_lenth, PM1_SENSOR, p->pm1);				//PM1.0
	cout << endl;

	Equipment_DATA_GET(obj, p);
	strcpy(p->ship_data, "notinfo");
	cout << " data_type:" << p->data_type << " ship_data:" << p->ship_data << endl;

	set_DATA_air(p);

	return;
}

void TrConnection::parse_3001_ship(const string& obj, struct SAP_DATA*& p)
{
	cout << "<<<<3001_ship data>>>>" << endl;

	SAP_DATA_GET(obj, water_lenth, WATER_TEMPER, p->WATER_TEMPER);		//水温
	SAP_DATA_GET(obj, water_lenth, DISSOLVED_OX, p->dissolved_ox);		//溶解氧
	SAP_DATA_GET(obj, water_lenth, NH3N_WATER, p->nh3n);				//氨氮	
	SAP_DATA_GET(obj, water_lenth, CUPRIC_ION, p->cupric_ion);			//铜离子
	SAP_DATA_GET(obj, water_lenth, CADMIUM_ION, p->cadmium_ion);		//镉离子
	SAP_DATA_GET(obj, water_lenth, BGPI_WATER, p->bg_algae);			//蓝绿藻
	SAP_DATA_GET(obj, water_lenth, PHYII_WATER, p->ph);					//叶绿素
	SAP_DATA_GET(obj, water_lenth, COND_WATER, p->conductivity);		//电导率
	SAP_DATA_GET(obj, water_lenth, TOC_WATER, p->toc);					//总有机碳（TDS）
	cout << endl;

	Equipment_DATA_GET(obj, p);
	strcpy(p->gas_data, "notinfo");
	cout << " data_type:" << p->data_type << " gas_data:" << p->gas_data << endl;

	set_DATA_3001(p);

	return;
}

void TrConnection::parse_car(const string& obj, struct SAP_DATA*& p)
{
	cout << "<<<<car data>>>>" << endl;

	SAP_DATA_GET(obj, gas_lenth, CO_SENSOR, p->co);					//一氧化氮
	SAP_DATA_GET(obj, gas_lenth, O3_SENSOR, p->o3);					//臭氧
	SAP_DATA_GET(obj, gas_lenth, SO2_SENSOR, p->so2);				//二氧化硫	
	SAP_DATA_GET(obj, gas_lenth, NO2_SENSOR, p->no2);				//二氧化氮
	SAP_DATA_GET(obj, gas_lenth, HUMID_SENSOR, p->hum);				//湿度
	SAP_DATA_GET(obj, gas_lenth, TEMPER_SENSOR, p->temper);			//温度
	SAP_DATA_GET(obj, gas_lenth, CO2_SENSOR, p->co2);				//二氧化碳
	SAP_DATA_GET(obj, gas_lenth, ILUM, p->luminous);					//光照
	//SAP_DATA_GET(obj, water_lenth, TOC_WATER, p->toc);					//总有机碳（TDS）
	cout << endl;

	Equipment_DATA_GET(obj, p);
	strcpy(p->ship_data, "notinfo");
	cout << " data_type:" << p->data_type << " gas_data:" << p->ship_data << endl;

	set_DATA_car(p);

	return;
}

void TrConnection::parse_6001_ship(const string& obj, struct SAP_DATA*& p)
{
	cout << "<<<<6001_ship data>>>>" << endl;
	cout << "<<<<Water quality data>>>>" << endl;

	SAP_DATA_GET(obj, water_lenth, DISSOLVED_OX, p->dissolved_ox);		//溶解氧
	SAP_DATA_GET(obj, water_lenth, NH3N_WATER, p->nh3n);				//氨氮
	SAP_DATA_GET(obj, water_lenth, BGPI_WATER, p->bg_algae);			//蓝绿藻
	SAP_DATA_GET(obj, water_lenth, PHYII_WATER, p->cod);				//叶绿素(借位COD）
	SAP_DATA_GET(obj, water_lenth, COND_WATER, p->conductivity);		//电导率
	SAP_DATA_GET(obj, water_lenth, TURB_WATER, p->turbidity);			//浊度
	SAP_DATA_GET(obj, water_lenth, PH_WATER, p->ph);					//PH
	SAP_DATA_GET(obj, water_lenth, WATER_TEMPER, p->WATER_TEMPER);		//水温
	cout << endl;

	cout << "<<<<Meteorological data>>>>" << endl;

	SAP_DATA_GET(obj, gas_lenth, CO_SENSOR, p->co);				//一氧化碳
	SAP_DATA_GET(obj, gas_lenth, O3_SENSOR, p->o3);					//臭氧
	SAP_DATA_GET(obj, gas_lenth, SO2_SENSOR, p->so2);				//二氧化硫
	SAP_DATA_GET(obj, gas_lenth, NO2_SENSOR, p->no2);				//二氧化氮
	SAP_DATA_GET(obj, gas_lenth, HUMID_SENSOR, p->hum);				//湿度
	SAP_DATA_GET(obj, gas_lenth, TEMPER_SENSOR, p->temper);				//气温
	SAP_DATA_GET(obj, gas_lenth, CO2_SENSOR, p->co2);				//二氧化碳
	SAP_DATA_GET(obj, gas_lenth, ILUM, p->luminous);					//光照

	cout << endl;

	Equipment_DATA_GET(obj, p);
	strcpy(p->gas_data, "notinfo");
	cout << " data_type:" << p->data_type << " gas_data:" << p->gas_data << endl;

	set_DATA_6001(p);

	return;
}

void TrConnection::parse_04(const string& meg)				//04包解析
{
	cout << meg << endl;
	cout << "<<<<04 Packet Analysis>>>>" << endl;														//04包分析
	int packetsize = meg.size();																		//获取04包的长度
	if (48 <= packetsize && packetsize <= 70)																//如果04包长度大于48且小于70
	{
		int IsrIp_len = 0;
		isr_mess* isr_mess_reg = new isr_mess;
		IsrIp_len = meg.size() - IsrIp_st - 1;															//04包长-48
		strcpy(isr_mess_reg->isr_net_id, (meg.substr(netid_st, netid_len)).c_str());					// 从输入字符串 meg 中提取特定字段，并使用 strcpy 复制到 isr_mess_reg 的成员中
		strcpy(isr_mess_reg->isr_data_len, (meg.substr(datalen_st, datalen_len)).c_str());
		strcpy(isr_mess_reg->isr_mac, (meg.substr(IsrMac_st, IsrMac_len)).c_str());
		strcpy(isr_mess_reg->isr_id, (meg.substr(IsrId_st, IsrId_len)).c_str());
		strcpy(isr_mess_reg->isr_gps, (meg.substr(IsrGps_st, IsrGps_len)).c_str());
		strcpy(isr_mess_reg->isr_cpu, (meg.substr(IsrCpu_st, IsrCpu_len)).c_str());
		strcpy(isr_mess_reg->isr_ram, (meg.substr(IsrRam_st, IsrRam_len)).c_str());
		strcpy(isr_mess_reg->isr_ip, (meg.substr(IsrIp_st, IsrIp_len)).c_str());
		strcpy(isr_mess_reg->isr_reg_time, getTime());

		cout << "isr_id:" << isr_mess_reg->isr_id << " isr_net_id:" << isr_mess_reg->isr_net_id << " isr_data_len: " << isr_mess_reg->isr_data_len << " isr_mac:" << isr_mess_reg->isr_mac << " isr_gps: " << isr_mess_reg->isr_gps << " isr_cpu: " << isr_mess_reg->isr_cpu << " isr_ram: " << isr_mess_reg->isr_ram << " isr_ip: " << isr_mess_reg->isr_ip << " isr_reg_time: " << isr_mess_reg->isr_reg_time << endl;

		//匹配套接字以及isr_mac
		device_match[isr_mess_reg->isr_mac] = clientfd_cur;											//将当前的客户端的文件描述符与isr_mac地址绑定
		cout << "current equipment: " << isr_mess_reg->isr_mac << " fd: " << clientfd_cur << endl;
		for (auto b : device_match) {									//遍历所有的键值对，（当遍历完所有键值对，空的值为0，for循环停止遍历）
			cout << b.first << " " << b.second << endl;					//it->first来访问键，使用it->second访问值
		}

		Trdb->handle_04(isr_mess_reg);									//处理04数据包：插入isr相关数据					
		delete isr_mess_reg;											//释放内存

		cout << "<<<<04 Packet Access>>>> " << endl;					//04包访问

		char* TS_buf = protocol_pack_10();								//生成一个指向10号协议的指针    //发送本地时间
		int is_send = send(clientfd_cur, TS_buf, strlen(TS_buf), MSG_DONTWAIT);			//发送数据
		if (-1 == is_send)												//判断发送失败
		{
			cout << "Reply 10 message failed!!" << endl;
		}
		else {
			cout << "Reply 10 message successfully!!" << endl;
		}
	}
	else																//如果包长不是大于48且小于70，则该包不是04包
	{
		cout << "<<<<IT IS not 04 Packet Access>>>> " << endl;
	}
}

void TrConnection::parse_05(const string& meg)							//05包解析
{
	cout << "<<<<05 Packet Analysis>>>>" << endl;
	int packetsize = meg.size();										//获取05包的长度
	if (packetsize == 71)												//05包为固定长度
	{
		sap_mess* sap_mess_reg = new sap_mess;
		strcpy(sap_mess_reg->sap_net_id, (meg.substr(netid_st, netid_len)).c_str());			// 从输入字符串 meg 中提取特定字段，并使用 strcpy 复制到 sap_mess_reg 的成员中
		strcpy(sap_mess_reg->sap_data_len, (meg.substr(datalen_st, datalen_len)).c_str());
		strcpy(sap_mess_reg->sap_isr_mac, (meg.substr(IsrMac_st, IsrMac_len)).c_str());
		strcpy(sap_mess_reg->sap_isr_id, (meg.substr(IsrId_st, IsrId_len)).c_str());
		strcpy(sap_mess_reg->sap_mac, (meg.substr(SapMac_st, SapMac_len)).c_str());
		strcpy(sap_mess_reg->sap_id, (meg.substr(SapId_st, SapId_len)).c_str());
		strcpy(sap_mess_reg->sap_gps, (meg.substr(SapGps_st, SapGps_len)).c_str());
		strcpy(sap_mess_reg->sap_cpu, (meg.substr(SapCpu_st, SapCpu_len)).c_str());
		strcpy(sap_mess_reg->sap_ram, (meg.substr(SapRam_st, SapRam_len)).c_str());
		strcpy(sap_mess_reg->sap_com_type, (meg.substr(SapComtype_st, SapComtype_len)).c_str());
		strcpy(sap_mess_reg->sap_port, (meg.substr(SapPort_st, SapPort_len)).c_str());
		strcpy(sap_mess_reg->sap_reg_time, getTime());
		//年月
		sap_mess_reg->year_mon[0] = sap_mess_reg->sap_reg_time[2];
		sap_mess_reg->year_mon[1] = sap_mess_reg->sap_reg_time[3];
		sap_mess_reg->year_mon[2] = sap_mess_reg->sap_reg_time[5];
		sap_mess_reg->year_mon[3] = sap_mess_reg->sap_reg_time[6];
		cout << "sap_isr_id:" << sap_mess_reg->sap_isr_id << " sap_data_len:" << sap_mess_reg->sap_data_len << " sap_id:" << sap_mess_reg->sap_id << " sap_isr_mac:" << sap_mess_reg->sap_isr_mac << " sap_mac:" << sap_mess_reg->sap_mac << endl;
		cout << " sap_gps:" << sap_mess_reg->sap_gps << " sap_com_type:" << sap_mess_reg->sap_com_type << " sap_port:" << sap_mess_reg->sap_port << " sap_reg_time:" << sap_mess_reg->sap_reg_time << endl;

		Trdb->handle_05(sap_mess_reg);											//处理05包：注册or升级更新SAP
		delete sap_mess_reg;													//释放

		cout << "<<<<05 Packet Access>>>> " << endl;
	}
	else {
		cout << "<<<<IT IS not 05 Packet Access>>>> " << endl;
	}
}

void TrConnection::parse_06(const string& meg)											//06包
{
	cout << "<<<<06 Packet Analysis>>>> " << endl;
	SAP_DATA* sap_data = new SAP_DATA;

	//私有头解析
	strcpy(sap_data->air_isr_mac, (meg.substr(IsrMac_06_st, IsrMac_06_len)).c_str());
	strcpy(sap_data->air_data_len, (meg.substr(datalen_st, datalen_len)).c_str());
	strcpy(sap_data->air_isr_id, (meg.substr(IsrId_06_st, IsrId_06_len)).c_str());
	strcpy(sap_data->air_sap_mac, (meg.substr(SapMac_06_st, SapMac_06_len)).c_str());
	strcpy(sap_data->air_sap_id, (meg.substr(SapId_06_st, SapId_06_len)).c_str());
	strcpy(sap_data->air_com_port, (meg.substr(SapPort_06_st, SapPort_06_len)).c_str());
	strcpy(sap_data->air_net_id, (meg.substr(netid_st, netid_len)).c_str());
	strcpy(sap_data->air_com_type, (meg.substr(SapComtype_06_st, SapComtype_06_len)).c_str());
	strcpy(sap_data->sap_cpu_rate, (meg.substr(SapCpu_06_st, SapCpu_06_len)).c_str());
	strcpy(sap_data->sap_ram_rate, (meg.substr(SapRam_06_st, SapRam_06_len)).c_str());

	cout << "air_isr_mac: " << sap_data->air_isr_mac << " air_isr_id:" << sap_data->air_isr_id << " air_sap_mac:" << sap_data->air_sap_mac << " air_sap_id:" << sap_data->air_sap_id << endl;
	cout << "<<message length>>: " << meg.length() << "<<Data area length>>: " << HextoDec(sap_data->air_data_len) << "(Dec) " << sap_data->air_data_len << "(Hex)" << endl;

	//匹配
	device_match[sap_data->air_isr_mac] = clientfd_cur;
	cout << "current equipment: " << sap_data->air_isr_mac << " fd: " << clientfd_cur << endl;
	for (auto a : device_match) {
		cout << a.first << " " << a.second << endl;
	}

	//数据出错
	if (meg.length() < HextoDec(sap_data->air_data_len) + 17)                                 //？？+17   11(hex)
	{
		cout << "Failed, the current packet is wrong..." << endl;
		delete sap_data;
		return;
	}

	//截取数据段
	int flag = -1;
	flag = meg.find("#");
	//解析数据段
	if ((meg.length() > HjGass_06_st) && (flag >= 0))
	{
		int ender = meg.find("@") - flag;//HJ212数据包的长度
		string temper_meg = meg.substr(flag, ender);
		cout << "HJ212: " << temper_meg << endl;

		//if (strcmp(sap_data->air_isr_id, "15") == 0)
		//{
		//	parse_water(temper_meg, sap_data);
		//	strcpy(sap_data->ship_data, temper_meg.c_str());//水质数据包
		//}
		//else if (strcmp(sap_data->air_isr_id, "04") == 0)
		//{
		//	parse_air(temper_meg, sap_data);
		//	strcpy(sap_data->gas_data, temper_meg.c_str());//南山空气数据包
		//}

		//此处由两个解析函数组成（后续可以考虑融合为一个）：分别对应逸夫楼顶设备、云阳中云趸大船设备
		//int iswater = temper_meg.find(WATER_TEMPER);		//通过看传过来的数据里面有没有WATER_TEMPER和SO2_SENSOR进行区分
		//int isair = temper_meg.find(SO2_SENSOR);

		//if (iswater != -1)
		//{
		//    parse_water(temper_meg, sap_data);
		//    strcpy(sap_data->ship_data, temper_meg.c_str());//水质数据包
		//}
		//else if (isair != -1)//南山空气数据
		//{
		//    parse_air(temper_meg, sap_data);
		//    strcpy(sap_data->gas_data, temper_meg.c_str());//南山空气数据包
		//}
		parse_DATA(temper_meg, sap_data);
	}
	else
	{
		//包不正常,后续可以考虑返回不正常包的信息
		cout << "Data packets of type 06 have no data information, Failed!" << endl;
		delete sap_data;
		return;
	}

	cout << "<<transfer time>>: " << sap_data->air_real_time << endl;
	sap_data->year[0] = sap_data->air_real_time[2];
	sap_data->year[1] = sap_data->air_real_time[3];
	sap_data->month[0] = sap_data->air_real_time[5];
	sap_data->month[1] = sap_data->air_real_time[6];
	cout << "<<current time>>:  " << getTime() << endl;

	Trdb->handle_06(sap_data);
	delete sap_data;

	cout << "<<<<06 Packet Access>>>> " << endl;
}

void TrConnection::parse_22(const string& meg)
{
	cout << "<<<<Online>>>> " << endl;
	Timer::TimerId time_id;
	string air_isr_mac1 = meg.substr(3, 16).c_str();
	device_match[air_isr_mac1] = clientfd_cur;
	cout << "current equipment: " << air_isr_mac1 << " fd: " << clientfd_cur << endl;
	for (auto a : device_match) {
		cout << a.first << " " << a.second << endl;
	}
	time_id = mEnv->scheduler()->resetTimerEvent(mtimeid, 15 * 60 * 1000);
	cout << "reset outtime and timeid: " << time_id << endl;
}

void TrConnection::parse_30()
{
	cout << "<<<<30 Packet>>>> " << endl;
}

void TrConnection::parse_31()
{
	cout << "<<<<31 Packet>>>> " << endl;
}

void TrConnection::parse_33(const string& meg)
{
	cout << "<<<<33 Packet Analysis>>>>" << endl;
	sap_data_33* sap_data = new sap_data_33;

	string path1;
	string co_33;
	string o3_33;
	string so2_33;
	string no2_33;
	string hum_33;
	string temper_33;
	string co2_33;
	string lux_33;

	int flag_start;


	//切割处理
	strcpy(sap_data->isr_id_new, (meg.substr(6, 2)).c_str());
	strcpy(sap_data->sap_id_new, (meg.substr(22, 2)).c_str());

	cout << "isr_id_new: " << sap_data->isr_id_new << endl;
	cout << "sap_id_new: " << sap_data->sap_id_new << endl;

	flag_start = meg.find(CO_SENSOR) + 11;
	co_33 = meg.substr(flag_start, 10);//co的数据
	sap_data->co_new = co_33.c_str();
	cout << "co: " << co_33 << endl;	

	flag_start = meg.find(O3_SENSOR) + 11;
	o3_33 = meg.substr(flag_start, 10);//o3的数据
	sap_data->o3_new = o3_33.c_str();
	cout << "o3: " << o3_33 << endl;

	flag_start = meg.find(SO2_SENSOR) + 11;
	so2_33 = meg.substr(flag_start, 10);//so2的数据
	sap_data->so2_new = so2_33.c_str();
	cout << "so2: " << so2_33 << endl;

	flag_start = meg.find(NO2_SENSOR) + 11;
	no2_33 = meg.substr(flag_start, 10);//no2的数据
	sap_data->no2_new = no2_33.c_str();
	cout << "no2: " << no2_33 << endl;

	flag_start = meg.find(HUMID_SENSOR) + 11;
	hum_33 = meg.substr(flag_start, 10);//湿度的数据
	sap_data->hum_new = hum_33.c_str();
	cout << "hum: " << hum_33 << endl;

	flag_start = meg.find(TEMPER_SENSOR) + 11;
	temper_33 = meg.substr(flag_start, 10);//温度的数据
	sap_data->temper_new = temper_33.c_str();
	cout << "temper: " << temper_33 << endl;

	flag_start = meg.find(CO2_SENSOR) + 11;
	co2_33 = meg.substr(flag_start, 10);//co2的数据
	sap_data->co2_new = co2_33.c_str();
	cout << "co2: " << co2_33 << endl;

	flag_start = meg.find(ILUM) + 11;//sensor_flag=11，正好为“a21026-Rtd=”的长度
	lux_33 = meg.substr(flag_start, 10);//光照的数据
	sap_data->luminous_new = lux_33.c_str();
	cout << "lux: " << lux_33 << endl;

	//SAP_DATA_GET(meg, water_lenth, CO_SENSOR, sap_data->co2_new);		//co
	//SAP_DATA_GET(meg, water_lenth, O3_SENSOR, sap_data->o3_new);				//o3
	//SAP_DATA_GET(meg, water_lenth, SO2_SENSOR, sap_data->so2_new);			//so2
	//SAP_DATA_GET(meg, water_lenth, NO2_SENSOR, sap_data->no2_new);				//no2
	//SAP_DATA_GET(meg, water_lenth, HUMID_SENSOR, sap_data->hum_new);		//湿度
	//SAP_DATA_GET(meg, water_lenth, TEMPER_SENSOR, sap_data->temper_new);			//温度
	//SAP_DATA_GET(meg, water_lenth, CO2_SENSOR, sap_data->co2_new);					//co2
	//SAP_DATA_GET(meg, water_lenth, ILUM, sap_data->luminous_new);		//光照

	flag_start = meg.find(PATH) + 8;
	path1 = meg.substr(flag_start, 2);//路径数据
	if (strcmp(sap_data->isr_id_new, path1.c_str()) == 0)
	{
		string kong;
		kong = '0';
		sap_data->path_new = kong.c_str();
		cout << "path: " << 0 << endl;
	}
	else
	{
		sap_data->path_new = path1.c_str();
		cout << "path: " << path1 << endl;
	}
	//sap_data->path_new = path1.c_str();
	//切割结束
	Trdb->handle_33(sap_data);

	delete sap_data;

	cout << "<<<<33 Packet Analysis>>>>" << endl;


}

