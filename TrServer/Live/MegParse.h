#pragma once
#include <map>
#include <iostream>
#include <string>

extern std::map<std::string, int> device_match;

//解析包头
#define ISR_REGIST     	 "04" 
#define AP_REGIST 	     "05"
#define SAP_TRANS_DATA	 "06"
#define ISR_TRANS_DATA	 "07"
#define AP_HISTORY_DATA  "08"
#define ISR_HEARTBEAT	 "09"
#define ISR_REPLY1		 "30"
#define ISR_REPLY2		 "31"
#define ISR_UPD_REGIST   "22"
#define ISR_CHANGE       "33"

// ISR和SAP公共部分的数据：切割字符信息
const int pct_st = 1, pct_len = 2;
const int netid_st = 6, netid_len = 4;
const int datalen_st = 12, datalen_len = 4;

// 04包-ISR设备注册数据：切割字符信息
const int IsrMac_st = 16, IsrMac_len = 16;
const int IsrId_st = 19, IsrId_len = 2;
const int IsrGps_st = 32, IsrGps_len = 11;
const int IsrCpu_st = 43, IsrCpu_len = 2;
const int IsrRam_st = 45, IsrRam_len = 2;
const int IsrIp_st = 47;

// 05包-sap设备注册数据：切割字符信息
// 05包的isr_mac与04包位置目前保持一致
const int SapMac_st = 32, SapMac_len = 16;
const int SapId_st = 35, SapId_len = 2;
const int SapGps_st = 48, SapGps_len = 11;
const int SapCpu_st = 59, SapCpu_len = 2;
const int SapRam_st = 61, SapRam_len = 2;
const int SapComtype_st = 63, SapComtype_len = 4;
const int SapPort_st = 67, SapPort_len = 3;

//06包-sap设备传输的数据：切割字符信息
const int IsrMac_06_st = 16, IsrMac_06_len = 16;
const int IsrId_06_st = 19, IsrId_06_len = 2;
const int SapMac_06_st = 32, SapMac_06_len = 16;
const int SapId_06_st = 35, SapId_06_len = 2;
const int SapPort_06_st = 48, SapPort_06_len = 3;
const int SapGps_06_st = 51, SapGps_06_len = 11;
const int SapCpu_06_st = 62, SapCpu_06_len = 2;
const int SapRam_06_st = 64, SapRam_06_len = 2;
const int SapComtype_06_st = 66, SapComtype_06_len = 4;
const int HjGass_06_st = 70, HjGass_06_len = 0;
const int HjSoil_06_st = 0, HjSoil_06_len = 0;
const int Hjwater_06_st = 0, Hjwater_06_len = 0;

//HJ212中Gps等数据
const std::string TIME_DEV = "QN=";
const std::string GPS_DEV = "Gps=";
const std::string MAC_DEV = "MN=";

// 大气数据HJ212数据头
const std::string SO2_SENSOR = "a21026-Rtd=";
const std::string O2_SENSOR = "a19001-Rtd=";
const std::string O3_SENSOR = "a19002-Rtd=";
const std::string CH4_SENSOR = "a05002-Rtd=";
const std::string NO_SENSOR = "a21003-Rtd=";
const std::string NH3_SENSOR = "a21001-Rtd=";
const std::string NO2_SENSOR = "a21004-Rtd=";
const std::string H2S_SENSOR = "a21028-Rtd=";
const std::string CO2_SENSOR = "a05001-Rtd=";
const std::string CO_SENSOR = "a21005-Rtd=";
const std::string TEMPER_SENSOR = "a01001-Rtd=";
const std::string HUMID_SENSOR = "a01002-Rtd=";
const std::string PM10_SENSOR = "a34002-Rtd=";
const std::string PM25_SENSOR = "a34004-Rtd=";
const std::string PM1_SENSOR = "a34005-Rtd=";

// 水质数据HJ212数据头
const std::string WATER_TEMPER = "w01010-Rtd=";	//水温
const std::string PH_WATER = "w01001-Rtd=";		//PH
const std::string COND_WATER = "w01014-Rtd=";	//电导率
const std::string TURB_WATER = "w01012-Rtd=";	//浊度
const std::string DISSOLVED_OX = "w01009-Rtd=";	//溶解氧
const std::string COD_WATER = "w00001-Rtd=";		//水质含氧量
const std::string TOC_WATER = "w01006-Rtd=";		//总有机碳  TDS(水中总溶解性物质的浓度)
const std::string NH3N_WATER = "w21003-Rtd=";	//氨氮
const std::string PHYII_WATER = "w00004-Rtd=";	//叶绿素
const std::string BGPI_WATER = "w00003-Rtd=";	//蓝绿藻
const std::string CUPRIC_ION = "w20138-Rtd=";    //铜离子
const std::string CADMIUM_ION = "w20143-Rtd=";   //镉离子
const std::string COD = "w01018-Rtd=";           //化学需氧量

// 水文数据HJ212数据头
const std::string VELO_WATER = "w00006-Rtd=";	//流速
const std::string DEPTH_WATER = "w00007-Rtd=";	//深度

// 气象数据HJ212数据头
const std::string WIND_DIR = "a01008-Rtd=";		//风向
const std::string WIND_SPE = "a01007-Rtd=";		//风速
const std::string WIND_SPE2M = "q00003-Rtd=";	//2分钟风速
const std::string WIND_SPE10M = "LA-Rtd=";		//噪音
const std::string AMBI_TEMP = "q00005-Rtd=";		//环境温度
const std::string MAX_TEMP = "q00006-Rtd=";		//最高温度
const std::string MIN_TEMP = "q00007-Rtd=";		//最低温度
const std::string AMBI_HUMI = "a01002-Rtd=";		//环境湿度
const std::string DEWP_HUMI = "q00009-Rtd=";		//露点湿度
const std::string AIR_PRES = "a01006-Rtd=";		//气压
const std::string ILUM = "a01004-Rtd=";			//光强度(光照)
const std::string RAIN_FALL = "a01003-Rtd=";		//雨量

//路径数据头
const std::string PATH = "from:";

//解析HJ212数据包
const int sensor_lenth = 10;			//无人船参数用的21位
const int gas_lenth = 8;				//大气参数用的19位
const int sensor_flag = 11;
const int time_lenth = 17;
const int time_mac_flag = 3;
const int gps_lenth = 20;
const int gps_flag = 4;
const int mac_lenth = 27;
const int water_lenth = 10;             //新增水质数据长度

struct SAP_DATA {
    char air_mac[30];           //大气采集设备的唯一标识符
    char air_id[4];             //大气采集设备的ID
    char air_sap_id[4];         //SAP设备的ID
    char air_sap_mac[20];       //SAP设备的MAC
    char air_isr_id[4];         //所属的isr设备ID
    char air_isr_mac[20];       //所属的isr设备的唯一标识符
    char air_net_id[10];        //网络ID
    char air_data_len[10];      //数据长度
    char air_com_time[64];      //大气采集设备的通信时间
    char air_real_time[64];     //系统目前的实际时间（当前北京时间）
    char air_com_type[6];       //通信采集设备与SAP设备的类型
    char air_com_port[6];       //RS485通信端口
    char data_type[6];			//数据类型
    char sap_gps[20];           //接入设备的GPS信息
    char sap_dev_gps[20];       //设备的GPS信息(获取HJ212里面的数据)
    char sap_cpu_rate[4];       //接入设备的cpu利用率信息
    char sap_ram_rate[4];       //接入设备的内存利用率信息
    char gas_data[2048];        //大气数据的数据包
    char ship_data[2048];       //船载设备数据的数据包
    char hj212_crc[10];			//HJ212数据包的循环冗余码
    char year[3];				//注册年份
    char month[3];				//注册月份
    double so2, h2s, nh3, no2, no, co, co2, o2, o3, ch4, ethanol1, ethanol2, ethanol3, pm10, pmD4, temper, hum, pm1;	//大气数据
    double WATER_TEMPER, ph, conductivity, turbidity, dissolved_ox, cod, toc, nh3n, chlorophyll, bg_algae, cupric_ion, cadmium_ion;//水质
    double flow_velocity, depth_water;//???
    double wind_direction, wind_speed, wind_speed_2m, wind_speed_10m, ambient_temp, max_temp, min_temp, ambient_humi, dewp_humi, air_press, luminous, rain_fall;//气象
};

struct sap_mess {
    char sap_id[4];				//接入设备ID
    char sap_isr_id[4];			//接入设备所属的isrID
    char sap_net_id[10];		//网络ID
    char sap_data_len[10];  	//数据长度,私有协议中
    char sap_mac[20];			//接入设备的mac信息
    char sap_isr_mac[20];		//所属的网关设备的mac信息
    char sap_gps[20];			//接入设备的GPS信息
    char sap_cpu[4];			//接入设备的cpu利用率信息
    char sap_ram[4];			//接入设备的内存利用率信息
    char sap_reg_time[64];		//接入设备的注册时间
    char sap_com_type[6];		//通信方式
    char sap_port[6];			//端口信息
    char year_mon[6];			//注册年月
};

struct isr_mess
{
    char isr_id[4] = "0";				//网关ID
    char isr_net_id[10] = "0";		//网络ID
    char isr_data_len[10] = "0";  	//数据长度
    char isr_mac[20] = "0";			//ISR的mac信息
    char isr_gps[20] = "0";			//ISR的GPS信息
    char isr_cpu[4] = "0";			//ISR的cpu利用率信息
    char isr_ram[4] = "0";			//ISR的内存利用率信息
    char isr_ip[20] = "0";			//ISR的IP地址
    char isr_reg_time[64] = "0";		//ISR的注册时间
};

struct sap_data_33
{
    char isr_id_new[20] = {};
    char sap_id_new[20] = {};
    const char* path_new;
    const char* co_new;
    const char* o3_new;
    const char* so2_new;
    const char* no2_new;
    const char* hum_new;
    const char* temper_new;
    const char* co2_new;
    const char* luminous_new;

    //char path_new[20];
    //char co_new[20];
    //char o3_new[20];
    //char so2_new[20];
    //char no2_new[20];
    //char hum_new[20];
    //char temper_new[20];
    //char co2_new[20];
    //char luminous_new[20];
    //double co_new, o3_new, so2_new, no2_new, hum_new, temper_new, co2_new, luminous_new;
};
