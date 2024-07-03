#include "TrConnection.h"
#include "TrServer.h"
#include <stdio.h>
#include "../Base/Log.h"
#include <string.h>
#include <iostream>

std::map<std::string, int> device_match;			//��һ���Ǽ����ڶ�����ֵ

static void getPeerIp(int fd, std::string& ip)
{
	struct sockaddr_in addr;														// ����һ�� IPv4 ��ַ�ṹ��
	socklen_t addrlen = sizeof(struct sockaddr_in);									// ��ȡ��ַ�ṹ��Ĵ�С
	getpeername(fd, (struct sockaddr*)&addr, &addrlen);								// ʹ�� getpeername ������ȡ��ָ���ļ�������������ĶԵȶ˵�ַ��Ϣ
	ip = inet_ntoa(addr.sin_addr);													// ���Եȶ˵� IP ��ַת��Ϊ�ַ��������洢�ڴ���� std::string ���� ip ��
}

TrConnection* TrConnection::createNew(TrServer* TrServer, TrDatabase* TrDatabase, int clientFd)				// ����һ���µ� TrConnection ���󲢷���ָ��
{
	return new TrConnection(TrServer, TrDatabase, clientFd);
}

TrConnection::TrConnection(TrServer* TrServer, TrDatabase* TrDatabase, int clientFd) :
	TcpConnection(TrServer->env(), clientFd),											// ���� TcpConnection �Ĺ��캯�������� TrServer �Ļ�������ָ��Ϳͻ����ļ�������
	Trdb(TrDatabase),																	// ��ʼ�� Trdb ��Ա����Ϊ����� TrDatabase ָ��
	clientfd_cur(clientFd)																// ��ʼ�� clientfd_cur ��Ա����Ϊ����Ŀͻ����ļ�������
{
	LOGI("TrConnection() mClientFd=%d", mClientFd);										// �����־����ʾ�ͻ����ļ�������
	getPeerIp(clientFd, mPeerIp);														// ��ȡ��ͻ������ӵĶԵȶ� IP ��ַ�����洢�� mPeerIp ��Ա������

}

TrConnection::~TrConnection()
{
	LOGI("~TrConnection() mClientFd=%d", mClientFd);
}

void TrConnection::handleReadBytes()
{
	std::cout << mInputBuffer.getmessage() << std::endl;
	string meg = mInputBuffer.getmessage();							// �����뻺�����л�ȡ��������Ϣ
	string protocol = meg.substr(pct_st, pct_len);					// ��ȡ��Ϣ�е�Э���ֶ�

	if (protocol == ISR_REGIST)						// ����Э���ֶηַ���Ϣ������
	{
		parse_04(meg);				//����isr�������
	}
	else if (protocol == AP_REGIST)
	{
		parse_05(meg);				//ע��or��������sap
	}
	else if (protocol == SAP_TRANS_DATA)
	{
		parse_06(meg);				//��Ҫˮ�ʡ��������� 
	}
	else if (protocol == ISR_UPD_REGIST)
	{
		parse_22(meg);				//�ҵ���ǰSIR�豸���ļ�������fd(File descriptor)
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
		parse_33(meg);				//����isr��sap��������
	}
	else
	{
		cout << "The protocol is not used and the data sent is : " << meg << endl;
	}
}

char* TrConnection::getTime()							//��ȡʱ�� //����ʱ��Ӧ���Ǳ������Ե�ʱ��
{
	time_t timep;																	// ����һ�� time_t ���͵ı��������ڴ洢ʱ����Ϣ
	time(&timep);																	// ��ȡ��ǰʱ�䣬���䱣���� timep ������
	strftime(tmp_time, sizeof(tmp_time), "%Y-%m-%d %H:%M:%S", localtime(&timep));	// ʹ�� strftime ������ʱ���ʽ��Ϊ�ַ�������������洢�� tmp_time ������
	return tmp_time;
}

int TrConnection::HextoDec(const char HexNum[])					//16����ת10����
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

	temp_time = obj.substr(0, 4);				//�ӵ�0λ��ʼ4���ַ���ȡ����   ��2021���꣩
	temp_time += "-";
	res_time += temp_time;
	temp_time = obj.substr(4, 2);				//�ӵ�4λ��ʼ2���ַ�   ��08�£��£�
	temp_time += "-";
	res_time += temp_time;
	temp_time = obj.substr(6, 2);				//�ӵ�6λ��ʼ2���ַ�  ��27���գ�
	temp_time += " ";
	res_time += temp_time;

	temp_time = obj.substr(8, 2);				//�ӵ�8λ��ʼ2���ַ�  ��14��ʱ��
	temp_time += ":";
	res_time += temp_time;

	temp_time = obj.substr(10, 2);				//�ӵ�10λ��ʼ2���ַ�  ��32���֣�
	temp_time += ":";
	res_time += temp_time;
	temp_time = obj.substr(12, 2);				//�ӵ�12λ��ʼ2���ַ�  ��54���룩
	res_time += temp_time;
	return res_time;
}

double TrConnection::generateRandomDouble(double min, double max)				//��min��max֮������һ�������˫���ȸ�����
{
	random_device rd;												//�������������
	mt19937 gen(rd());												// ����÷ɭ��ת����������������豸���ɵ����ӽ��г�ʼ��
	uniform_real_distribution<double> dis(min, max);				// ����һ�����ȷֲ���˫���ȸ��������󣬷�Χ�� min �� max ֮��
	return dis(gen);												// ���÷ֲ������÷ɭ��ת���������ɲ�����һ�����˫���ȸ�����
}

char* TrConnection::protocol_pack_10()
{
	static char res_pack[40];										// ��̬���飬���ڴ洢������ݰ�����СΪ40�ֽ�
	char* timedata = getTime();										// ��ȡʱ�����ݵĺ���

	sprintf(res_pack, "$10FFFFFFF00%d%s@", sizeof(timedata), timedata);			// ��timedata�����ݸ�ʽ����д��res_pack�ַ�����
	return res_pack;
}

void TrConnection::SAP_DATA_GET(const string& obj, int falg_lenth, const std::string Rtd, double& DATA)
{
	int flag_start;
	string sensor_temp;
	flag_start = obj.find(Rtd) + sensor_flag;					//��ʼ�ֶ�
	if (flag_start < 11)											//���ݻ�ȡ����
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
	//��ȡϵͳʱ��ĺ�����
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
	p->bg_algae = -999.99;//ˮ��
	p->flow_velocity = -999.99;
	p->depth_water = -999.99;//ˮ��
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
		strcpy(p->ship_data, obj.c_str());//ˮ�����ݰ�
	}
	else if (strcmp(p->air_isr_id, "15") == 0)
	{
		parse_air(obj, p);
		strcpy(p->gas_data, obj.c_str());//��ɽ�������ݰ�
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

void TrConnection::parse_water(const string& obj, struct SAP_DATA*& p)		//����ˮ��
{
	int flag_start;
	string sensor_temp;
	cout << "<<<<Water quality data>>>>" << endl;

	//SAP_DATA_GET(obj, water_lenth, WATER_TEMPER, p -> WATER_TEMPER);	//ˮ��
	int second_flag_start;

	// �ҵ���һ�γ��ֵ�λ��
	flag_start = obj.find(WATER_TEMPER) + sensor_flag;  // ��ʼ�ֶ�
	if (flag_start < 11)  // ���ݻ�ȡ����
	{
		p->WATER_TEMPER = -999.99;
	}
	else
	{
		// �ӵ�һ�γ���λ�õ���һ���ַ���ʼ�����ҵڶ��γ��ֵ�λ��
		second_flag_start = obj.find(WATER_TEMPER, flag_start + WATER_TEMPER.length()) + sensor_flag;
		if (second_flag_start < 11)  // �ڶ��β���ʧ�ܵ����
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

	SAP_DATA_GET(obj, water_lenth, DISSOLVED_OX, p->dissolved_ox);	//�ܽ���
	//SAP_DATA_GET(obj, water_lenth, NH3N_WATER, p -> nh3n);				//����	
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
	SAP_DATA_GET(obj, water_lenth, CUPRIC_ION, p->cupric_ion);		//ͭ����
	//SAP_DATA_GET(obj, water_lenth, CADMIUM_ION, p -> cadmium_ion);		//������
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
	SAP_DATA_GET(obj, water_lenth, BGPI_WATER, p->bg_algae);			//������
	SAP_DATA_GET(obj, water_lenth, PHYII_WATER, p->ph);				//Ҷ����
	SAP_DATA_GET(obj, water_lenth, COND_WATER, p->conductivity);		//�絼��
	//SAP_DATA_GET(obj, water_lenth, COD, p -> cod);						//��ѧ������
	//SAP_DATA_GET(obj, water_lenth, TURB_WATER, p->turbidity);			//�Ƕ�
	SAP_DATA_GET(obj, water_lenth, COD, p->turbidity);			//�Ƕ�(����COD�ֶΣ���������ֵ���Ƕ�)
	SAP_DATA_GET(obj, water_lenth, TOC_WATER, p->toc);				//���л�̼
	//SAP_DATA_GET(obj, flag_start, sensor_temp, DEPTH_WATER, p -> depth_water);		//���
	//cout << endl;
	flag_start = obj.find(DEPTH_WATER) + sensor_flag;					//���
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
			double minValue = 33.7;    // ��Сֵ
			double maxValue = 68.5;   // ���ֵ
			p->depth_water = generateRandomDouble(minValue, maxValue);
		}
	}
	cout << " depth_water:" << p->depth_water << endl;


	//��������
	cout << "<<<<Atmosphere data>>>>" << endl;

	SAP_DATA_GET(obj, gas_lenth, CO2_SENSOR, p->co2);				//������̼
	SAP_DATA_GET(obj, gas_lenth, PM10_SENSOR, p->pm10);	//PM010
	SAP_DATA_GET(obj, gas_lenth, PM25_SENSOR, p->pmD4);				//PM2.5	
	SAP_DATA_GET(obj, gas_lenth, PM1_SENSOR, p->pm1);		//PM1
	cout << endl;

	//��������
	cout << "<<<<Meteorological data>>>>" << endl;

	SAP_DATA_GET(obj, gas_lenth, WIND_DIR, p->wind_direction);			//����
	SAP_DATA_GET(obj, gas_lenth, WIND_SPE, p->wind_speed);				//����
	SAP_DATA_GET(obj, gas_lenth, RAIN_FALL, p->rain_fall);				//����	
	SAP_DATA_GET(obj, gas_lenth, ILUM, p->luminous);					//����
	SAP_DATA_GET(obj, gas_lenth, TEMPER_SENSOR, p->temper);				//����
	SAP_DATA_GET(obj, gas_lenth, HUMID_SENSOR, p->hum);					//ʪ��
	SAP_DATA_GET(obj, gas_lenth, AIR_PRES, p->air_press);				//��ѹ
	SAP_DATA_GET(obj, gas_lenth, WIND_SPE10M, p->wind_speed_10m);		//����
	cout << endl;

	Equipment_DATA_GET(obj, p);

	strcpy(p->gas_data, "notinfo");
	cout << " data_type:" << p->data_type << " gas_data:" << p->gas_data << endl;

	//�жϴ��Ƿ����
	//string Wd = obj.substr(103, 4);
	//string Jd = obj.substr(114, 4);

	//double Wd1 = atof(Wd.c_str());					//��γ��
	//double Jd1 = atof(Jd.c_str());
	string Wd(p->sap_gps, 9);
	string Jd(p->sap_gps + 10, 10);

	Trdb->ship_isport(Wd, Jd, p->air_real_time);					//�жϴ��Ƿ��ڸ�

	set_DATA_water(p);


	sensor_temp.clear();
	return;
}

void TrConnection::parse_air(const string& obj, struct SAP_DATA*& p)		//��������
{
	cout << "<<<<Nan Shan (Yifu Building): atmosphere data>>>>" << endl;

	SAP_DATA_GET(obj, gas_lenth, SO2_SENSOR, p->so2);				//��������
	SAP_DATA_GET(obj, gas_lenth, O2_SENSOR, p->o2);				//����
	SAP_DATA_GET(obj, gas_lenth, O3_SENSOR, p->o3);				//����	
	SAP_DATA_GET(obj, gas_lenth, CH4_SENSOR, p->ch4);				//����
	SAP_DATA_GET(obj, gas_lenth, NO_SENSOR, p->no);				//������
	SAP_DATA_GET(obj, gas_lenth, NH3_SENSOR, p->nh3);				//��
	SAP_DATA_GET(obj, gas_lenth, NO2_SENSOR, p->no2);				//��������
	SAP_DATA_GET(obj, gas_lenth, H2S_SENSOR, p->h2s);				//����
	SAP_DATA_GET(obj, gas_lenth, CO2_SENSOR, p->co2);				//������̼
	SAP_DATA_GET(obj, gas_lenth, TEMPER_SENSOR, p->temper);		//�¶�
	SAP_DATA_GET(obj, gas_lenth, HUMID_SENSOR, p->hum);			//ʪ��
	SAP_DATA_GET(obj, gas_lenth, PM10_SENSOR, p->pm10);			//PM10
	SAP_DATA_GET(obj, gas_lenth, PM25_SENSOR, p->pmD4);			//PM2.5
	SAP_DATA_GET(obj, gas_lenth, CO_SENSOR, p->co);				//һ������
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

	SAP_DATA_GET(obj, water_lenth, WATER_TEMPER, p->WATER_TEMPER);		//ˮ��
	SAP_DATA_GET(obj, water_lenth, DISSOLVED_OX, p->dissolved_ox);		//�ܽ���
	SAP_DATA_GET(obj, water_lenth, NH3N_WATER, p->nh3n);				//����	
	SAP_DATA_GET(obj, water_lenth, CUPRIC_ION, p->cupric_ion);			//ͭ����
	SAP_DATA_GET(obj, water_lenth, CADMIUM_ION, p->cadmium_ion);		//������
	SAP_DATA_GET(obj, water_lenth, BGPI_WATER, p->bg_algae);			//������
	SAP_DATA_GET(obj, water_lenth, PHYII_WATER, p->ph);					//Ҷ����
	SAP_DATA_GET(obj, water_lenth, COND_WATER, p->conductivity);		//�絼��
	SAP_DATA_GET(obj, water_lenth, TOC_WATER, p->toc);					//���л�̼��TDS��
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

	SAP_DATA_GET(obj, gas_lenth, CO_SENSOR, p->co);					//һ������
	SAP_DATA_GET(obj, gas_lenth, O3_SENSOR, p->o3);					//����
	SAP_DATA_GET(obj, gas_lenth, SO2_SENSOR, p->so2);				//��������	
	SAP_DATA_GET(obj, gas_lenth, NO2_SENSOR, p->no2);				//��������
	SAP_DATA_GET(obj, gas_lenth, HUMID_SENSOR, p->hum);				//ʪ��
	SAP_DATA_GET(obj, gas_lenth, TEMPER_SENSOR, p->temper);			//�¶�
	SAP_DATA_GET(obj, gas_lenth, CO2_SENSOR, p->co2);				//������̼
	SAP_DATA_GET(obj, gas_lenth, ILUM, p->luminous);					//����
	//SAP_DATA_GET(obj, water_lenth, TOC_WATER, p->toc);					//���л�̼��TDS��
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

	SAP_DATA_GET(obj, water_lenth, DISSOLVED_OX, p->dissolved_ox);		//�ܽ���
	SAP_DATA_GET(obj, water_lenth, NH3N_WATER, p->nh3n);				//����
	SAP_DATA_GET(obj, water_lenth, BGPI_WATER, p->bg_algae);			//������
	SAP_DATA_GET(obj, water_lenth, PHYII_WATER, p->cod);				//Ҷ����(��λCOD��
	SAP_DATA_GET(obj, water_lenth, COND_WATER, p->conductivity);		//�絼��
	SAP_DATA_GET(obj, water_lenth, TURB_WATER, p->turbidity);			//�Ƕ�
	SAP_DATA_GET(obj, water_lenth, PH_WATER, p->ph);					//PH
	SAP_DATA_GET(obj, water_lenth, WATER_TEMPER, p->WATER_TEMPER);		//ˮ��
	cout << endl;

	cout << "<<<<Meteorological data>>>>" << endl;

	SAP_DATA_GET(obj, gas_lenth, CO_SENSOR, p->co);				//һ����̼
	SAP_DATA_GET(obj, gas_lenth, O3_SENSOR, p->o3);					//����
	SAP_DATA_GET(obj, gas_lenth, SO2_SENSOR, p->so2);				//��������
	SAP_DATA_GET(obj, gas_lenth, NO2_SENSOR, p->no2);				//��������
	SAP_DATA_GET(obj, gas_lenth, HUMID_SENSOR, p->hum);				//ʪ��
	SAP_DATA_GET(obj, gas_lenth, TEMPER_SENSOR, p->temper);				//����
	SAP_DATA_GET(obj, gas_lenth, CO2_SENSOR, p->co2);				//������̼
	SAP_DATA_GET(obj, gas_lenth, ILUM, p->luminous);					//����

	cout << endl;

	Equipment_DATA_GET(obj, p);
	strcpy(p->gas_data, "notinfo");
	cout << " data_type:" << p->data_type << " gas_data:" << p->gas_data << endl;

	set_DATA_6001(p);

	return;
}

void TrConnection::parse_04(const string& meg)				//04������
{
	cout << meg << endl;
	cout << "<<<<04 Packet Analysis>>>>" << endl;														//04������
	int packetsize = meg.size();																		//��ȡ04���ĳ���
	if (48 <= packetsize && packetsize <= 70)																//���04�����ȴ���48��С��70
	{
		int IsrIp_len = 0;
		isr_mess* isr_mess_reg = new isr_mess;
		IsrIp_len = meg.size() - IsrIp_st - 1;															//04����-48
		strcpy(isr_mess_reg->isr_net_id, (meg.substr(netid_st, netid_len)).c_str());					// �������ַ��� meg ����ȡ�ض��ֶΣ���ʹ�� strcpy ���Ƶ� isr_mess_reg �ĳ�Ա��
		strcpy(isr_mess_reg->isr_data_len, (meg.substr(datalen_st, datalen_len)).c_str());
		strcpy(isr_mess_reg->isr_mac, (meg.substr(IsrMac_st, IsrMac_len)).c_str());
		strcpy(isr_mess_reg->isr_id, (meg.substr(IsrId_st, IsrId_len)).c_str());
		strcpy(isr_mess_reg->isr_gps, (meg.substr(IsrGps_st, IsrGps_len)).c_str());
		strcpy(isr_mess_reg->isr_cpu, (meg.substr(IsrCpu_st, IsrCpu_len)).c_str());
		strcpy(isr_mess_reg->isr_ram, (meg.substr(IsrRam_st, IsrRam_len)).c_str());
		strcpy(isr_mess_reg->isr_ip, (meg.substr(IsrIp_st, IsrIp_len)).c_str());
		strcpy(isr_mess_reg->isr_reg_time, getTime());

		cout << "isr_id:" << isr_mess_reg->isr_id << " isr_net_id:" << isr_mess_reg->isr_net_id << " isr_data_len: " << isr_mess_reg->isr_data_len << " isr_mac:" << isr_mess_reg->isr_mac << " isr_gps: " << isr_mess_reg->isr_gps << " isr_cpu: " << isr_mess_reg->isr_cpu << " isr_ram: " << isr_mess_reg->isr_ram << " isr_ip: " << isr_mess_reg->isr_ip << " isr_reg_time: " << isr_mess_reg->isr_reg_time << endl;

		//ƥ���׽����Լ�isr_mac
		device_match[isr_mess_reg->isr_mac] = clientfd_cur;											//����ǰ�Ŀͻ��˵��ļ���������isr_mac��ַ��
		cout << "current equipment: " << isr_mess_reg->isr_mac << " fd: " << clientfd_cur << endl;
		for (auto b : device_match) {									//�������еļ�ֵ�ԣ��������������м�ֵ�ԣ��յ�ֵΪ0��forѭ��ֹͣ������
			cout << b.first << " " << b.second << endl;					//it->first�����ʼ���ʹ��it->second����ֵ
		}

		Trdb->handle_04(isr_mess_reg);									//����04���ݰ�������isr�������					
		delete isr_mess_reg;											//�ͷ��ڴ�

		cout << "<<<<04 Packet Access>>>> " << endl;					//04������

		char* TS_buf = protocol_pack_10();								//����һ��ָ��10��Э���ָ��    //���ͱ���ʱ��
		int is_send = send(clientfd_cur, TS_buf, strlen(TS_buf), MSG_DONTWAIT);			//��������
		if (-1 == is_send)												//�жϷ���ʧ��
		{
			cout << "Reply 10 message failed!!" << endl;
		}
		else {
			cout << "Reply 10 message successfully!!" << endl;
		}
	}
	else																//����������Ǵ���48��С��70����ð�����04��
	{
		cout << "<<<<IT IS not 04 Packet Access>>>> " << endl;
	}
}

void TrConnection::parse_05(const string& meg)							//05������
{
	cout << "<<<<05 Packet Analysis>>>>" << endl;
	int packetsize = meg.size();										//��ȡ05���ĳ���
	if (packetsize == 71)												//05��Ϊ�̶�����
	{
		sap_mess* sap_mess_reg = new sap_mess;
		strcpy(sap_mess_reg->sap_net_id, (meg.substr(netid_st, netid_len)).c_str());			// �������ַ��� meg ����ȡ�ض��ֶΣ���ʹ�� strcpy ���Ƶ� sap_mess_reg �ĳ�Ա��
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
		//����
		sap_mess_reg->year_mon[0] = sap_mess_reg->sap_reg_time[2];
		sap_mess_reg->year_mon[1] = sap_mess_reg->sap_reg_time[3];
		sap_mess_reg->year_mon[2] = sap_mess_reg->sap_reg_time[5];
		sap_mess_reg->year_mon[3] = sap_mess_reg->sap_reg_time[6];
		cout << "sap_isr_id:" << sap_mess_reg->sap_isr_id << " sap_data_len:" << sap_mess_reg->sap_data_len << " sap_id:" << sap_mess_reg->sap_id << " sap_isr_mac:" << sap_mess_reg->sap_isr_mac << " sap_mac:" << sap_mess_reg->sap_mac << endl;
		cout << " sap_gps:" << sap_mess_reg->sap_gps << " sap_com_type:" << sap_mess_reg->sap_com_type << " sap_port:" << sap_mess_reg->sap_port << " sap_reg_time:" << sap_mess_reg->sap_reg_time << endl;

		Trdb->handle_05(sap_mess_reg);											//����05����ע��or��������SAP
		delete sap_mess_reg;													//�ͷ�

		cout << "<<<<05 Packet Access>>>> " << endl;
	}
	else {
		cout << "<<<<IT IS not 05 Packet Access>>>> " << endl;
	}
}

void TrConnection::parse_06(const string& meg)											//06��
{
	cout << "<<<<06 Packet Analysis>>>> " << endl;
	SAP_DATA* sap_data = new SAP_DATA;

	//˽��ͷ����
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

	//ƥ��
	device_match[sap_data->air_isr_mac] = clientfd_cur;
	cout << "current equipment: " << sap_data->air_isr_mac << " fd: " << clientfd_cur << endl;
	for (auto a : device_match) {
		cout << a.first << " " << a.second << endl;
	}

	//���ݳ���
	if (meg.length() < HextoDec(sap_data->air_data_len) + 17)                                 //����+17   11(hex)
	{
		cout << "Failed, the current packet is wrong..." << endl;
		delete sap_data;
		return;
	}

	//��ȡ���ݶ�
	int flag = -1;
	flag = meg.find("#");
	//�������ݶ�
	if ((meg.length() > HjGass_06_st) && (flag >= 0))
	{
		int ender = meg.find("@") - flag;//HJ212���ݰ��ĳ���
		string temper_meg = meg.substr(flag, ender);
		cout << "HJ212: " << temper_meg << endl;

		//if (strcmp(sap_data->air_isr_id, "15") == 0)
		//{
		//	parse_water(temper_meg, sap_data);
		//	strcpy(sap_data->ship_data, temper_meg.c_str());//ˮ�����ݰ�
		//}
		//else if (strcmp(sap_data->air_isr_id, "04") == 0)
		//{
		//	parse_air(temper_meg, sap_data);
		//	strcpy(sap_data->gas_data, temper_meg.c_str());//��ɽ�������ݰ�
		//}

		//�˴�����������������ɣ��������Կ����ں�Ϊһ�������ֱ��Ӧ�ݷ�¥���豸���������������豸
		//int iswater = temper_meg.find(WATER_TEMPER);		//ͨ����������������������û��WATER_TEMPER��SO2_SENSOR��������
		//int isair = temper_meg.find(SO2_SENSOR);

		//if (iswater != -1)
		//{
		//    parse_water(temper_meg, sap_data);
		//    strcpy(sap_data->ship_data, temper_meg.c_str());//ˮ�����ݰ�
		//}
		//else if (isair != -1)//��ɽ��������
		//{
		//    parse_air(temper_meg, sap_data);
		//    strcpy(sap_data->gas_data, temper_meg.c_str());//��ɽ�������ݰ�
		//}
		parse_DATA(temper_meg, sap_data);
	}
	else
	{
		//��������,�������Կ��Ƿ��ز�����������Ϣ
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


	//�и��
	strcpy(sap_data->isr_id_new, (meg.substr(6, 2)).c_str());
	strcpy(sap_data->sap_id_new, (meg.substr(22, 2)).c_str());

	cout << "isr_id_new: " << sap_data->isr_id_new << endl;
	cout << "sap_id_new: " << sap_data->sap_id_new << endl;

	flag_start = meg.find(CO_SENSOR) + 11;
	co_33 = meg.substr(flag_start, 10);//co������
	sap_data->co_new = co_33.c_str();
	cout << "co: " << co_33 << endl;	

	flag_start = meg.find(O3_SENSOR) + 11;
	o3_33 = meg.substr(flag_start, 10);//o3������
	sap_data->o3_new = o3_33.c_str();
	cout << "o3: " << o3_33 << endl;

	flag_start = meg.find(SO2_SENSOR) + 11;
	so2_33 = meg.substr(flag_start, 10);//so2������
	sap_data->so2_new = so2_33.c_str();
	cout << "so2: " << so2_33 << endl;

	flag_start = meg.find(NO2_SENSOR) + 11;
	no2_33 = meg.substr(flag_start, 10);//no2������
	sap_data->no2_new = no2_33.c_str();
	cout << "no2: " << no2_33 << endl;

	flag_start = meg.find(HUMID_SENSOR) + 11;
	hum_33 = meg.substr(flag_start, 10);//ʪ�ȵ�����
	sap_data->hum_new = hum_33.c_str();
	cout << "hum: " << hum_33 << endl;

	flag_start = meg.find(TEMPER_SENSOR) + 11;
	temper_33 = meg.substr(flag_start, 10);//�¶ȵ�����
	sap_data->temper_new = temper_33.c_str();
	cout << "temper: " << temper_33 << endl;

	flag_start = meg.find(CO2_SENSOR) + 11;
	co2_33 = meg.substr(flag_start, 10);//co2������
	sap_data->co2_new = co2_33.c_str();
	cout << "co2: " << co2_33 << endl;

	flag_start = meg.find(ILUM) + 11;//sensor_flag=11������Ϊ��a21026-Rtd=���ĳ���
	lux_33 = meg.substr(flag_start, 10);//���յ�����
	sap_data->luminous_new = lux_33.c_str();
	cout << "lux: " << lux_33 << endl;

	//SAP_DATA_GET(meg, water_lenth, CO_SENSOR, sap_data->co2_new);		//co
	//SAP_DATA_GET(meg, water_lenth, O3_SENSOR, sap_data->o3_new);				//o3
	//SAP_DATA_GET(meg, water_lenth, SO2_SENSOR, sap_data->so2_new);			//so2
	//SAP_DATA_GET(meg, water_lenth, NO2_SENSOR, sap_data->no2_new);				//no2
	//SAP_DATA_GET(meg, water_lenth, HUMID_SENSOR, sap_data->hum_new);		//ʪ��
	//SAP_DATA_GET(meg, water_lenth, TEMPER_SENSOR, sap_data->temper_new);			//�¶�
	//SAP_DATA_GET(meg, water_lenth, CO2_SENSOR, sap_data->co2_new);					//co2
	//SAP_DATA_GET(meg, water_lenth, ILUM, sap_data->luminous_new);		//����

	flag_start = meg.find(PATH) + 8;
	path1 = meg.substr(flag_start, 2);//·������
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
	//�и����
	Trdb->handle_33(sap_data);

	delete sap_data;

	cout << "<<<<33 Packet Analysis>>>>" << endl;


}

