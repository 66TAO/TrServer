#include "TrDatabase.h"



TrDatabase* TrDatabase::createNew()
{
	return new TrDatabase();
}

TrDatabase::TrDatabase()
{
	LOGI("TrDatabase");
}

//TrDatabase::~TrDatabase() {};

bool TrDatabase::init_db()																			// 初始化数据库连接
{
	mysql_init(&db_g2020);
	if (!(mysql_real_connect(&db_g2020, MYSQL_IP, MYSQL_USER, MYSQL_PWD, MYSQL_NAME, 0, NULL, 0)))		// 尝试连接到数据库
	{
		printf("Error connecting to database:%s\n", mysql_error(&db_g2020));
		return false;
	}
	else
	{
		printf("Database connected...\n");
		memset(st_query, 0, sizeof(st_query));										// 检查是否存在 isr_total 表
		strcpy(st_query, "select * from isr_total;");
		state = mysql_query(&db_g2020, st_query);				//向与指定的连接标识符关联的服务器中的当前活动数据库发送一条查询
		if (0 == state)																// 查询成功，说明 isr_total 表存在，打印提示信息，释放查询结果并返回 true
		{
			printf("isr_total is in Ship_DB!!\n");
			res = mysql_use_result(&db_g2020);						//获取查询的结果集
			mysql_free_result(res);									//释放查询的结果集
			return true;
		}
		else
		{
			printf("isr_total does not exist：%s\n", mysql_error(&db_g2020));		// 查询失败，说明 isr_total 表不存在，打印错误信息并返回 false
			return false;
		}
	}
}

void TrDatabase::insert_total(struct isr_mess*& p)							// 插入 isr_total 表记录
{
	memset(st_query, 0, sizeof(st_query));										// 清空查询缓存
	// 构造 SQL 查询语句，使用 sprintf 将变量值填充到查询字符串中
	sprintf((char*)st_query, "INSERT INTO isr_total(isr_id,net_id,isr_ip,isr_mac,isr_gps,isr_regist_time,isr_com_time,isr_cpu_rate,isr_ram_rate) VALUE(\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\');", p->isr_id, p->isr_net_id, p->isr_ip, p->isr_mac, p->isr_id, p->isr_reg_time, p->isr_reg_time, p->isr_cpu, p->isr_ram);						//向名为 isr_total 的表中插入 ISR 的各种信息// 构造插入 isr_total 表记录的 SQL 语句
	temp_sql = st_query;
	//LOGI("SQL: %s", temp_sql);
	std::cout << "SQL:" << temp_sql << std::endl;
	state = mysql_query(&db_g2020, st_query);
	if (0 == state)
	{
		memset(st_query, 0, sizeof(st_query));									// 插入成功，创建对应的 isr_00xx 表
		sprintf((char*)st_query, "CREATE TABLE isr_00%s(sap_id char(2) NOT NULL,sap_reg_ym char(4) NOT NULL,sap_mac char(16) NULL,sap_isr_id char(2) NULL,sap_isr_mac char(16) NULL,net_id char(4) NULL,sap_gps char(11) NULL,sap_register_time varchar(24) NULL,sap_com_time varchar(24) NULL,com_type char(4) NULL,port_type char(3) NULL,sap_cpu_rate char(2) NULL,sap_ram_rate char(2) NULL,PRIMARY KEY(sap_id,sap_reg_ym)) ENGINE=InnoDB;", p->isr_id);//建表：isr_00（p->isr_id）// 构造创建 isr_00xx 表的 SQL 语句

		temp_sql = st_query;
		std::cout << "SQL:" << temp_sql << std::endl;
		state = mysql_query(&db_g2020, st_query);

		if (0 == state)
		{
			std::cout << "Insert isr_total and creat isr_00" << p->isr_id << " Success!!" << std::endl;				// 创建表成功
		}
		else
		{
			std::cout << "Create table isr_00" << p->isr_id << " failed:" << mysql_error(&db_g2020) << std::endl;		// 创建表失败，打印错误信息
		}
	}
	else
	{
		std::cout << "Insert isr_total failed:" << mysql_error(&db_g2020) << std::endl;						// 插入失败，打印错误信息
	}
}

void TrDatabase::update_total(struct isr_mess*& p)					//更新表中数据	
{
	res = mysql_use_result(&db_g2020);									// 使用 mysql_use_result() 函数获取查询结果
	mysql_free_result(res);												// 释放查询结果集内存，以免造成内存泄漏
	memset(st_query, 0, sizeof(st_query));								// 清空查询缓存


	sprintf((char*)st_query, "UPDATE isr_total SET net_id = \'%s\',isr_ip= \'%s\',isr_gps= \'%s\',isr_regist_time= \'%s\',isr_com_time= \'%s\',isr_cpu_rate= \'%s\',isr_ram_rate= \'%s\' WHERE isr_id = \'%s\';", \
		p->isr_net_id, p->isr_ip, p->isr_gps, p->isr_reg_time, p->isr_reg_time, p->isr_cpu, p->isr_ram, p->isr_id);			// 构造 SQL 查询语句，使用 UPDATE 关键字更新表中的数据（两个isr_reg_time？？）

	state = mysql_query(&db_g2020, st_query);							// 执行 SQL 查询
	temp_sql = st_query;
	std::cout << "SQL:" << temp_sql << std::endl;

	if (0 == state)														// 检查查询状态
	{
		std::cout << "Update isr_total Success!!" << std::endl;			// 输出成功消息
		res = mysql_use_result(&db_g2020);								// 获取查询结果
		mysql_free_result(res);											// 释放查询结果内存
	}
	else
	{
		std::cout << "update_total failed:" << mysql_error(&db_g2020) << std::endl;
	}
}

void TrDatabase::insert_isr(struct sap_mess*& p)								//插入isr_00%s和创建sap_%s%s_%s表
{
	memset(st_query, 0, sizeof(st_query));																	// 清空查询缓存
	sprintf((char*)st_query, "INSERT INTO isr_00%s(sap_id,sap_reg_ym,sap_mac,sap_isr_id,sap_isr_mac,net_id,sap_gps,sap_register_time,sap_com_time,com_type,port_type,sap_cpu_rate,sap_ram_rate) \
    	VALUE(\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\');", \
		p->sap_isr_id, p->sap_id, p->year_mon, p->sap_mac, p->sap_isr_id, p->sap_isr_mac, p->sap_net_id, p->sap_net_id, p->sap_reg_time, p->sap_reg_time, p->sap_com_type, p->sap_port, p->sap_cpu, p->sap_ram);
	state = mysql_query(&db_g2020, st_query);								// 执行 SQL 查询

	if (0 == state)
	{
		//创建表sap000x
		memset(st_query, 0, sizeof(st_query));
		sprintf((char*)st_query, "CREATE TABLE sap_%s%s_%s(sap_index INT NOT NULL AUTO_INCREMENT,air_mac char(27) NULL,air_id char(2) NULL,air_sap_id char(2) NULL,air_sap_mac char(16) NULL,air_isr_id char(2) NULL,air_isr_mac char(16) NULL,\
			air_net_id char(4) NULL,air_com_type char(4) NULL,sap_dev_gps char(20) NULL,air_com_port char(3) NULL,air_com_time varchar(24) NULL,air_real_time varchar(24) NULL,sap_cpu_rate char(2) NULL,sap_ram_rate char(2) NULL,data_type char(3) NULL,\
			gas_data varchar(2048) NULL,ship_data varchar(2048) NULL,\
			so2 double(12,6) NULL,h2s double(12,6) NULL,nh3 double(12,6) NULL,no2 double(12,6) NULL,no double(12,6) NULL,co double(12,6) NULL,co2 double(12,6) NULL,\
			o2 double(12,6) NULL,o3 double(12,6) NULL,ch4 double(12,6) NULL,pm10 double(12,6) NULL,pm25 double(12,6) NULL,temp double(12,6) NULL,hum double(12,6) NULL,\
			WATER_TEMPER double(12,6) NULL,ph double(12,6) NULL,conductivity double(12,6) NULL,turbidity double(12,6) NULL,dissolved_ox double(12,6) NULL,cod double(12,6) NULL,toc double(12,6) NULL,nh3n double(12,6) NULL,chlorophyll double(12,6) NULL,bg_algae double(12,6) NULL,\
			flow_velocity double(12,6) NULL,depth_water double(12,6) NULL,\
			wind_direction double(12,6) NULL,wind_speed double(12,6) NULL,wind_speed_2m double(12,6) NULL,wind_speed_10m double(12,6) NULL,ambient_temp double(12,6) NULL,max_temp double(12,6) NULL,\
			min_temp double(12,6) NULL,ambient_humi double(12,6) NULL,dewp_humi double(12,6) NULL,air_press double(12,6) NULL,luminous double(12,6) NULL,rain_fall double(12,6) NULL,\
			PRIMARY KEY(sap_index)) ENGINE=InnoDB;", p->sap_isr_id, p->sap_id, p->year_mon);

		state = mysql_query(&db_g2020, st_query);							// 执行创建表的 SQL 查询该表用来存储气体浓度温度等
		if (0 == state)
		{
			std::cout << "Create table sap_" << p->sap_isr_id << p->sap_id << "_" << p->year_mon << " Success!" << std::endl;			// 输出成功消息
		}
		else
		{
			std::cout << "Create table sap_" << p->sap_isr_id << p->sap_id << "failed: " << mysql_error(&db_g2020) << std::endl;			// 输出创建表失败消息和错误信息
		}
	}
	else
	{
		std::cout << "Insert table isr_00" << p->sap_isr_id << "failed: " << mysql_error(&db_g2020) << std::endl;			// 输出插入记录失败消息和错误信息
	}
}

void TrDatabase::update_isr(struct sap_mess*& p)				//更新isr_00%s		
{
	memset(st_query, 0, sizeof(st_query));												// 清空查询缓存	
	sprintf((char*)st_query, "UPDATE isr_00%s SET sap_id = \'%s\',sap_mac= \'%s\',sap_isr_id= \'%s\',sap_isr_mac= \'%s\',net_id= \'%s\',sap_gps=\'%s\',sap_register_time= \'%s\',sap_com_time= \'%s\',\
    	com_type= \'%s\',port_type= \'%s\',sap_cpu_rate= \'%s\',sap_ram_rate= \'%s\' WHERE sap_id = \'%s\' AND sap_reg_ym = \'%s\';", \
		p->sap_isr_id, p->sap_id, p->sap_mac, p->sap_isr_id, p->sap_isr_mac, p->sap_net_id, p->sap_net_id, p->sap_reg_time, p->sap_reg_time, p->sap_com_type, p->sap_port, p->sap_cpu, p->sap_ram, p->sap_id, p->year_mon);

	state = mysql_query(&db_g2020, st_query);											// 执行 SQL 查询

	if (0 == state)
	{
		std::cout << "Update table isr_00" << p->sap_isr_id << " Success!" << std::endl;
		res = mysql_use_result(&db_g2020);
		mysql_free_result(res);
	}
	else
	{
		std::cout << "Update table isr_00" << p->sap_isr_id << " Failed: " << mysql_error(&db_g2020) << std::endl;
	}
}

void TrDatabase::insert_sap(struct SAP_DATA*& q)			//插入06数据包的SAP数据
{
	// 插入记录
	memset(st_query, 0, sizeof(st_query));
	std::cout << "gps: " << q->sap_gps << std::endl;
	sprintf((char*)st_query, "INSERT INTO sap_%s%s_%c%c%c%c(air_mac,air_id,air_sap_id,air_sap_mac,air_isr_id,air_isr_mac,air_net_id,air_com_type,air_com_port,air_com_time,air_real_time,sap_dev_gps,sap_cpu_rate,sap_ram_rate,\
    	data_type,gas_data,ship_data,so2,h2s,nh3,no2,no,co,co2,o2,o3,ch4,pm10,pm25,temp,hum,WATER_TEMPER,ph,conductivity,turbidity,dissolved_ox,cod,toc,nh3n,chlorophyll,bg_algae,\
    	flow_velocity,depth_water,wind_direction,wind_speed,wind_speed_2m,wind_speed_10m,ambient_temp,max_temp,min_temp,ambient_humi,dewp_humi,air_press,luminous,rain_fall) \
    	VALUE(\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\
    	\'%f\',\'%f\',\'%f\',\'%f\',\'%f\',\'%f\',\'%f\',\'%f\',\'%f\',\'%f\',\'%f\',\'%f\',\'%f\',\'%f\',\'%f\',\'%f\',\'%f\',\'%f\',\'%f\',\'%f\',\'%f\',\'%f\',\'%f\',\'%f\',\'%f\',\'%f\',\'%f\',\'%f\',\'%f\',\'%f\',\'%f\',\'%f\',\'%f\',\'%f\',\'%f\',\'%f\',\'%f\',\'%f\');", \
		q->air_isr_id, q->air_sap_id, q->year[0], q->year[1], q->month[0], q->month[1], q->air_mac, q->air_id, q->air_sap_id, q->air_sap_mac, q->air_isr_id, q->air_isr_mac, q->air_net_id, q->air_com_type, q->air_com_port, q->air_com_time, q->air_real_time, q->sap_gps, q->sap_cpu_rate, q->sap_ram_rate, q->data_type, q->gas_data, q->ship_data, \
		q->so2, q->h2s, q->nh3, q->no2, q->no, q->co, q->co2, q->o2, q->o3, q->ch4, q->pm10, q->pmD4, q->pm1, q->hum, \
		q->WATER_TEMPER, q->ph, q->conductivity, q->turbidity, q->dissolved_ox, q->cod, q->toc, q->nh3n, q->temper, q->bg_algae, q->cupric_ion, q->cadmium_ion, q->depth_water, q->wind_speed, q->wind_direction, q->wind_speed_10m, q->ambient_temp, q->max_temp, q->min_temp, q->ambient_humi, q->dewp_humi, q->air_press, q->luminous, q->rain_fall);

	temp_sql = st_query;
	state = mysql_query(&db_g2020, st_query);

	if (0 == state)
	{
		//数据插入成功，再更新isr_total和isr_00xx表的信息
		std::cout << "INSERT INTO sap_" << q->air_isr_id << q->air_sap_id << "_" << q->year[0] << q->year[1] << q->month[0] << q->month[1] << " Success!" << std::endl;

		//先更新isr_00xx的记录：net_id、sap_gps、sap_com_time、com_type、port_type、sap_cpu_rate、sap_ram_rate
		memset(st_query, 0, sizeof(st_query));
		sprintf((char*)st_query, "UPDATE isr_00%s SET net_id= \'%s\',sap_gps= \'%s\',sap_com_time= \'%s\',com_type= \'%s\',port_type= \'%s\',sap_cpu_rate= \'%s\',sap_ram_rate= \'%s\' WHERE sap_id = \'%s\' AND sap_reg_ym = \'%s%s\';"\
			, q->air_isr_id, q->air_net_id, q->air_net_id, q->air_com_time, q->air_com_type, q->air_com_port, q->sap_cpu_rate, q->sap_ram_rate, q->air_sap_id, q->year, q->month);

		state = mysql_query(&db_g2020, st_query);

		if (0 == state)
		{
			std::cout << "UPDATE table isr_00" << q->air_isr_id << " & isr_total Success!" << std::endl;
			res = mysql_use_result(&db_g2020);
			mysql_free_result(res);
		}
		else
		{
			std::cout << "UPDATE table isr_00" << q->air_isr_id << "Failed: " << mysql_error(&db_g2020) << std::endl;
		}
	}
	//插入失败
	else
	{
		std::cout << "INSERT INTO " << "sap_" << q->air_isr_id << q->air_sap_id << " failed: " << mysql_error(&db_g2020) << std::endl;
	}
	return;
}

void TrDatabase::sendmail(std::string Message)
{
	std::string from = "15608177097@163.com";
	std::string passs = "ZMDNRMDFNZEHYNNB";//这里替换成自己的授权码
	std::string to = "1213915399@qq.com";
	std::string subject = "Tr Softcenter";
	std::string strMessage = Message;


	std::vector<std::string> vecTo; //发送列表
	vecTo.push_back("1213915399@qq.com");

	std::vector<std::string> ccList;
	ccList.push_back("1598118780@qq.com");//抄送列表
	ccList.push_back("2271780427@qq.com");

	std::vector<std::string> attachment;
	/*attachment.push_back("mail.h");
	attachment.push_back("/home/soft/Desktop/openwrt-sdk-imx6ul/1/mybuild.sh");*/


	SmtpBase* base;
	/*SimpleSmtpEmail m_mail("smtp.163.com", "25");
	base = &m_mail;*/
	//base->SendEmail(from, passs, to, subject, strMessage);//普通的文本发送，明文发送

	SimpleSslSmtpEmail m_ssl_mail("smtp.163.com", "465");
	base = &m_ssl_mail;
	//base->SendEmail(from, passs, to, subject, strMessage);
	base->SendEmail(from, passs, vecTo, subject, strMessage, attachment, ccList);//加密的发送，支持抄送、附件等
}

void TrDatabase::ship_isport(std::string Wd, std::string Jd, const char* time)
{
	std::cout << "Wd: " << Wd << " " << "Jd: " << Jd << std::endl;											// 打印传入的经度和纬度

	long long Wd_val = std::stoll(Wd);						//****************
	long long Jd_val = std::stoll(Jd);
	if (305585489 < Wd_val && Wd_val < 305595698 && 1084123925 < Jd_val && Jd_val < 1084133666)
	{
		std::cout << "Ship in port!!!" << std::endl;															// 如果在指定范围内，打印提示信息：船只在港口内
		if (flag == 1)
		{
			sendmail("Your ship is in port");
			flag = 0;
		}
		memset(st_query, 0, sizeof(st_query));																	// 清空查询缓存
		sprintf((char*)st_query, "INSERT INTO ship_state(time, ship) VALUE(\'%s\',\'%f\');", time, 0.0);		// 构造插入船只状态表的 SQL 查询语句
		state = mysql_query(&db_g2020, st_query);																// 执行 SQL 查询
		temp_sql = st_query;																					// 将查询语句保存到临时变量中
		if (0 == state)																							// 根据查询执行结果输出相应的信息
		{
			std::cout << "INSERT INTO ship_state Success!" << std::endl;
		}
		else
		{
			std::cout << "INSERT INTO ship_state Failed: " << mysql_error(&db_g2020) << std::endl;
		}
	}
	else {
		std::cout << "Ship out of port!!!" << std::endl;														// 如果不在指定范围内，打印提示信息：船只不在港口内
		if (flag == 0)
		{
			sendmail("Your ship has cleared the port");
			flag = 1;
		}
		memset(st_query, 0, sizeof(st_query));																	// 清空查询缓存
		sprintf((char*)st_query, "INSERT INTO ship_state(time, ship) VALUE(\'%s\',\'%f\');", time, 1.0);		// 构造插入船只状态表的 SQL 查询语句
		state = mysql_query(&db_g2020, st_query);																// 执行 SQL 查询
		temp_sql = st_query;																					// 将查询语句保存到临时变量中
		if (0 == state)																							// 根据查询执行结果输出相应的信息
		{
			std::cout << "INSERT INTO ship_state Success!" << std::endl;
		}
		else
		{
			std::cout << "INSERT INTO ship_state Failed: " << mysql_error(&db_g2020) << std::endl;
		}
	}
}

void TrDatabase::handle_04(struct isr_mess*& isr_mess_reg)
{
	memset(st_query, 0, sizeof(st_query));																		// 清空查询缓存st_query里面全部赋值为0
	sprintf((char*)st_query, "SELECT * FROM isr_total WHERE isr_id = \'%s\';", isr_mess_reg->isr_id);			// 构建 SQL 查询语句，查询是否存在与 isr_id 匹配的记录
	state = mysql_query(&db_g2020, st_query);																	// 执行 SQL 查询
	temp_sql = st_query;																						// 保存查询语句，用于输出调试信息
	std::cout << "SQL: " << temp_sql << std::endl;
	if (0 == state)																								// 查询成功
	{
		res = mysql_use_result(&db_g2020);																		//获取查询结果集
		//（当使用mysql_use_result时，你必须执行mysql_fetch_row直到NULL值返回。否则那些没有被获取的列将作为你下个请求的一部分返回）
		if (mysql_fetch_row(res) == NULL)											// 如果查询结果为空，则插入新记录
		{
			mysql_free_result(res);													//释放
			insert_total(isr_mess_reg);												//将isr相关信息写入isr_total表中
		}
		else
		{
			mysql_free_result(res);													//释放
			update_total(isr_mess_reg);												//插入获取的数据
		}
	}
	else																			//查询不成功
	{
		std::cout << "select error：" << mysql_error(&db_g2020) << std::endl;		//打印错误描述
		if (0 != mysql_ping(&db_g2020))									// 尝试重新连接数据库
		{
			mysql_close(&db_g2020);										//关闭数据库
			init_db();													//初始化数据库
		}
	}
}

void TrDatabase::handle_05(struct sap_mess*& sap_mess_reg)
{
	memset(st_query, 0, sizeof(st_query));																// 清空查询缓存 st_query里面全部赋值为0
	sprintf((char*)st_query, "SELECT *FROM isr_total WHERE isr_id = \'%s\';", sap_mess_reg->sap_isr_id);
	state = mysql_query(&db_g2020, st_query);															// 执行 SQL 查询
	temp_sql = st_query;																				// 保存查询语句，用于输出调试信息
	std::cout << "SQL:" << temp_sql << std::endl;
	if (0 == state)																						//查询成功
	{
		res = mysql_use_result(&db_g2020);																//获取查询结果集
		if (mysql_fetch_row(res) == NULL)																//如果查询结果为空
		{
			mysql_free_result(res);																		//释放
			std::cout << "Failed The ISR device to which the SAP device belongs is not registered..." << std::endl;			//未注册
		}
		else
		{
			mysql_free_result(res);																		//释放

			memset(st_query, 0, sizeof(st_query));														// 清空查询缓存st_query里面全部赋值为0
			sprintf((char*)st_query, "UPDATE isr_total SET net_id = \'%s\',isr_com_time= \'%s\' WHERE isr_id = \'%s\';", sap_mess_reg->sap_net_id, sap_mess_reg->sap_reg_time, sap_mess_reg->sap_isr_id);//更新注册时间
			state = mysql_query(&db_g2020, st_query);													// 执行 SQL 查询
			temp_sql = st_query;
			if (0 == state)
			{
				std::cout << "UPDATE isr_total success..." << std::endl;								//更新成功		
			}

			memset(st_query, 0, sizeof(st_query));														// 清空查询缓存st_query里面全部赋值为0	
			sprintf((char*)st_query, "SELECT *FROM isr_00%s WHERE sap_id = \'%s\' AND sap_reg_ym = \'%s\';", sap_mess_reg->sap_isr_id, sap_mess_reg->sap_id, sap_mess_reg->year_mon);
			state = mysql_query(&db_g2020, st_query);													// 执行 SQL 查询
			temp_sql = st_query;
			if (0 == state)																				//如果查询结果为0
			{
				res = mysql_use_result(&db_g2020);														//获取查询结果集
				if (mysql_fetch_row(res) == NULL)
				{
					mysql_free_result(res);																//释放
					insert_isr(sap_mess_reg);															//注册ISR
				}
				else
				{
					mysql_free_result(res);																//释放
					update_isr(sap_mess_reg);															//升级ISR
				}
			}
			else
			{
				std::cout << "Failed,The ISR device is not registered..." << std::endl;				//错误这个ISR设备没有注册
			}
		}
	}
	else
	{
		//查询失败，返回错误信息
		std::cout << "select isr_total error：" << mysql_error(&db_g2020) << std::endl;
		if (0 != mysql_ping(&db_g2020))
		{
			mysql_close(&db_g2020);
			init_db();
		}
	}
}

void TrDatabase::handle_06(struct SAP_DATA*& sap_data)
{
	memset(st_query, 0, sizeof(st_query));
	sprintf((char*)st_query, "SELECT * FROM isr_total WHERE isr_id = \'%s\';", sap_data->air_isr_id);
	state = mysql_query(&db_g2020, st_query);
	temp_sql = st_query;
	if (0 == state)
	{
		res = mysql_use_result(&db_g2020);
		//对isr_total表查询，没有查询到isr_id的记录并且没有结果集，说明isr设备未成功注册(如果没有注册，后期考虑回传私有协议包传达：isr设备未成功注册的信息)
		if (mysql_fetch_row(res) == NULL)
		{
			mysql_free_result(res);
			std::cout << "Failed,The ISR device to which the AIR device belongs is not registered!!" << std::endl;
		}
		//对isr_total表查询，查询到isr_id的记录且有结果集,更新isr_total表的数据信息：isr_com_time
		else
		{
			mysql_free_result(res);
			memset(st_query, 0, sizeof(st_query));
			sprintf((char*)st_query, "SELECT *FROM isr_00%s WHERE sap_id = \'%s\' AND sap_reg_ym = \'%c%c%c%c\';", sap_data->air_isr_id, sap_data->air_sap_id, sap_data->year[0], sap_data->year[1], sap_data->month[0], sap_data->month[1]);
			state = mysql_query(&db_g2020, st_query);    //寻找isr_00xx表是否存在
			temp_sql = st_query;
			std::cout << "SQL: " << temp_sql << std::endl;

			if (0 == state)//成功执行查询语句
			{
				res = mysql_use_result(&db_g2020);//获取查询语句的结果集
				//查询成功，再查找对应的sap_id是否存在，存在则直接更新isr_00xx表的数据信息：net_id、sap_gps、sap_com_time、com_type、data_type、sap_cpu_rate、sap_ram_rate
				//未查询到sap_id='sap_data->air_sap_id'的记录
				if (NULL == mysql_fetch_row(res))
				{
					mysql_free_result(res);//释放结果集
					// 如果没找到这个表的数据，尝试找找上个月的该表是否存在，如果存在就新建表，如sap_0102_2205找不到，可以尝试找一下sap_0102_2204,找得到就建表sap_0102_2205
					int mon = (sap_data->month[0] - '0') * 10 + (sap_data->month[1] - '0');
					int temp_year = (sap_data->year[0] - '0') * 10 + (sap_data->year[1] - '0');

					char pre_mon[3];//去定义一个数组。
					if (mon < 10)//10月份之前
					{
						if (mon == 1)//1月份
						{
							mon = 12;
							temp_year -= 1;
							//itoa(mon,pre_mon,10);
							pre_mon[0] = '1';
							pre_mon[1] = '2';
						}
						else
						{
							mon -= 1;
							pre_mon[0] = '0';
							pre_mon[1] = mon + '0';
						}
					}
					else//10月份往后
					{
						if (mon == 10)
						{
							pre_mon[0] = '0';
							pre_mon[1] = '9';
						}
						else
						{
							mon -= 11;
							pre_mon[0] = '1';
							pre_mon[1] = mon + '0';
						}
					}
					std::cout << "<<<<Find last month's data>>>> year and month:" << temp_year << pre_mon << std::endl;
					memset(st_query, 0, sizeof(st_query));
					sprintf((char*)st_query, "SELECT *FROM isr_00%s WHERE sap_id = \'%s\' AND sap_reg_ym = \'%d%c%c\';", sap_data->air_isr_id, sap_data->air_sap_id, temp_year, pre_mon[0], pre_mon[1]);
					state = mysql_query(&db_g2020, st_query);    //寻找isr_00xx表是否存在
					temp_sql = st_query;
					std::cout << "SQL: " << temp_sql << std::endl;

					if (0 == state)
					{
						res = mysql_use_result(&db_g2020);
						if (NULL != mysql_fetch_row(res)) //表明上个月的表存在。
						{
							mysql_free_result(res);
							// 表示上个月的表存在，则准备建新表
							memset(st_query, 0, sizeof(st_query));
							sprintf((char*)st_query, "CREATE TABLE sap_%s%s_%c%c%c%c(sap_index INT NOT NULL AUTO_INCREMENT,air_mac char(27) NULL,air_id char(2) NULL,air_sap_id char(2) NULL,air_sap_mac char(16) NULL,air_isr_id char(2) NULL,air_isr_mac char(16) NULL,\
								    		air_net_id char(4) NULL,air_com_type char(4) NULL,sap_dev_gps char(20) NULL,air_com_port char(3) NULL,air_com_time varchar(24) NULL,air_real_time varchar(24) NULL,sap_cpu_rate char(2) NULL,sap_ram_rate char(2) NULL,data_type char(3) NULL,\
								    		gas_data varchar(2048) NULL,ship_data varchar(2048) NULL,\
								    		so2 double(12,6) NULL,h2s double(12,6) NULL,nh3 double(12,6) NULL,no2 double(12,6) NULL,no double(12,6) NULL,co double(12,6) NULL,co2 double(12,6) NULL,\
								    		o2 double(12,6) NULL,o3 double(12,6) NULL,ch4 double(12,6) NULL,pm10 double(12,6) NULL,pm25 double(12,6) NULL,temp double(12,6) NULL,hum double(12,6) NULL,\
								    		WATER_TEMPER double(12,6) NULL,ph double(12,6) NULL,conductivity double(12,6) NULL,turbidity double(12,6) NULL,dissolved_ox double(12,6) NULL,cod double(12,6) NULL,toc double(12,6) NULL,nh3n double(12,6) NULL,chlorophyll double(12,6) NULL,bg_algae double(12,6) NULL,\
								    		flow_velocity double(12,6) NULL,depth_water double(12,6) NULL,\
								    		wind_direction double(12,6) NULL,wind_speed double(12,6) NULL,wind_speed_2m double(12,6) NULL,wind_speed_10m double(12,6) NULL,ambient_temp double(12,6) NULL,max_temp double(12,6) NULL,\
								    		min_temp double(12,6) NULL,ambient_humi double(12,6) NULL,dewp_humi double(12,6) NULL,air_press double(12,6) NULL,luminous double(12,6) NULL,rain_fall double(12,6) NULL,\
								    		PRIMARY KEY(sap_index)) ENGINE=InnoDB;", sap_data->air_isr_id, sap_data->air_sap_id, sap_data->year[0], sap_data->year[1], sap_data->month[0], sap_data->month[1]);


							state = mysql_query(&db_g2020, st_query);
							temp_sql = st_query;
							if (0 == state)
							{
								std::cout << "Create table sap_" << sap_data->air_isr_id << sap_data->air_sap_id << "_" << sap_data->year << sap_data->month << " Success!" << std::endl;
							}
							else
							{
								std::cout << "Create table sap_" << sap_data->air_isr_id << sap_data->air_sap_id << "_" << sap_data->year << sap_data->month << " Failed: " << mysql_error(&db_g2020) << std::endl;
								delete sap_data;
								return;
							}

							// 创建成功，在isr_00xx中插入数据
							memset(st_query, 0, sizeof(st_query));


							//进行isr00%s表中的gps的数值进行一个判断。

							sprintf((char*)st_query, "INSERT INTO isr_00%s(sap_id,sap_reg_ym,sap_mac,sap_isr_id,sap_isr_mac,net_id,sap_gps,sap_com_time,com_type,port_type,sap_cpu_rate,sap_ram_rate) \
									    	VALUE(\'%s\',\'%s%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\');", \
								sap_data->air_isr_id, sap_data->air_sap_id, sap_data->year, sap_data->month, sap_data->air_sap_mac, sap_data->air_isr_id, sap_data->air_isr_mac, sap_data->air_net_id, \
								sap_data->sap_gps, sap_data->air_com_time, sap_data->air_com_type, sap_data->air_com_port, sap_data->sap_cpu_rate, sap_data->sap_ram_rate);

							std::cout << "sap_gps: " << sap_data->sap_gps << std::endl;
							state = mysql_query(&db_g2020, st_query);
							temp_sql = st_query;
							if (0 == state)
							{
								std::cout << "INSERT INTO isr_00" << sap_data->air_isr_id << " Success!" << std::endl;
							}
							else
							{
								std::cout << "INSERT INTO isr_00" << sap_data->air_isr_id << " Failed: " << mysql_error(&db_g2020) << std::endl;
								delete sap_data;
								return;
							}
							//插入数据到新表
							char sap_table_name[10];
							sap_table_name[0] = sap_data->air_isr_id[0];//0
							sap_table_name[1] = sap_data->air_isr_id[1];//1
							sap_table_name[2] = sap_data->air_sap_id[0];//0
							sap_table_name[3] = sap_data->air_sap_id[1];//2
							sap_table_name[4] = '_';
							sap_table_name[5] = sap_data->year[0];//2
							sap_table_name[6] = sap_data->year[1];//2
							sap_table_name[7] = sap_data->month[0];//0
							sap_table_name[8] = sap_data->month[1];//7


							insert_sap(sap_data);
							//append_time(sap_data->air_real_time, sap_table_name);//2022-4-11 09:15:45,0102_2207
							delete sap_data;
							return;
						}
						else
						{
							mysql_free_result(res);
							std::cout << "Failed,The SAP device to which the AIR device belongs is not registered!!" << std::endl;
							delete sap_data;
							return;
						}

					}
					else
					{
						// 表示isr_00xx表不存在，丢弃，只允许通过注册来建表
						std::cout << "Failed,The SAP device to which the AIR device belongs is not registered!!" << std::endl;
						delete sap_data;
						return;
					}
					//未找到(后期可以考虑回传私有协议包转达：sap设备未成功注册的信息)
				}
				//查询到了且有结果集
				else
				{
					//找到sap_xxyy表，在表sap_xxyy插入数据 xx为isr_id,yy为sap_id
					char sap_table_name[10];
					sap_table_name[0] = sap_data->air_isr_id[0];
					sap_table_name[1] = sap_data->air_isr_id[1];
					sap_table_name[2] = sap_data->air_sap_id[0];
					sap_table_name[3] = sap_data->air_sap_id[1];
					sap_table_name[4] = '_';
					sap_table_name[5] = sap_data->year[0];
					sap_table_name[6] = sap_data->year[1];
					sap_table_name[7] = sap_data->month[0];
					sap_table_name[8] = sap_data->month[1];

					mysql_free_result(res);

					insert_sap(sap_data);
					//append_time(sap_data->air_com_time, sap_table_name);
				}
			}
			else
			{
				//isr_000x表不存在，建立表isr_000x表，这个时候要考虑为什么isr注册的时候没有建表？
			}
		}
	}
	else
	{
		std::cout << "select error：" << mysql_error(&db_g2020) << std::endl;
		if (0 != mysql_ping(&db_g2020))
		{
			mysql_close(&db_g2020);
			init_db();
		}
	}

}

void TrDatabase::handle_33(struct sap_data_33*& sap_data)
{
	memset(st_query, 0, sizeof(st_query));
	//sprintf((char*)st_query, "INSERT INTO test_table(isr_id, sap_id, path, co2, hum, temper)VALUE(\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\');",isr_id_new, sap_id_new, path_new, co2_new, hum_new, temper_new);
	//if (strcmp(sap_data->isr_id_new, "19") == 0)
	//{
	//	sprintf((char*)st_query, "UPDATE isr19 SET isr_id = \'%s\',sap_id= \'%s\',path= \'%s\',co2= \'%s\',hum= \'%s\',temper= \'%s\';", \
	//		sap_data->isr_id_new, sap_data->sap_id_new, sap_data->path_new, sap_data->co2_new, sap_data->hum_new, sap_data->temper_new);
	//}
	//else if (strcmp(sap_data->isr_id_new, "18") == 0)
	//{
	//	sprintf((char*)st_query, "UPDATE isr18 SET isr_id = \'%s\',sap_id= \'%s\',path= \'%s\',co2= \'%s\',hum= \'%s\',temper= \'%s\';", \
	//		sap_data->isr_id_new, sap_data->sap_id_new, sap_data->path_new, sap_data->co2_new, sap_data->hum_new, sap_data->temper_new);
	//}
	//if (!sap_data->isr_id_new || !sap_data->sap_id_new || !sap_data->path_new || !sap_data->co_new ||
	//	!sap_data->o3_new || !sap_data->so2_new || !sap_data->no2_new || !sap_data->hum_new ||
	//	!sap_data->temper_new || !sap_data->co2_new || !sap_data->luminous_new) {
	//	std::cerr << "Error: One or more sap_data fields are null." << std::endl;
	//	return;
	//}
	sprintf((char*)st_query, "UPDATE test_table SET isr_id = \'%s\',sap_id= \'%s\',path= \'%s\',co= \'%s\',o3= \'%s\',so2= \'%s\',no2= \'%s\',hum= \'%s\',temper= \'%s\',co2= \'%s\',lux= \'%s\';", \
		sap_data->isr_id_new, sap_data->sap_id_new, sap_data->path_new, sap_data->co_new, sap_data->o3_new, sap_data->so2_new, sap_data->no2_new, sap_data->hum_new, sap_data->temper_new, sap_data->co2_new, sap_data->luminous_new);
	state = mysql_query(&db_g2020, st_query);
	temp_sql = st_query;
	std::cout << "sql: " << temp_sql << std::endl;

	if (0 == state)
	{
		std::cout << "update test_table success" << std::endl;
	}
	else
	{
		std::cout << "update test_table Failed: " << mysql_error(&db_g2020) << std::endl;
	}
}

void TrDatabase::read_wheather(struct SAP_DATA*& sap_data)
{
	memset(st_query, 0, sizeof(st_query));
	sprintf((char*)st_query, "SELECT temperature,pressure,humidity,precipitation,wind_speed FROM weather_records_yun WHERE station_id = 57339;");
	state = mysql_query(&db_g2020, st_query);
	temp_sql = st_query;
	if (0 == state)
	{
		res = mysql_use_result(&db_g2020);
		if (res == NULL) {
			LOGE(mysql_error(&db_g2020));
			return;
		}
		while ((row = mysql_fetch_row(res)) != NULL) {
			sap_data->temper = atof(row[0]);
			sap_data->air_press = atof(row[1]);
			sap_data->hum = atof(row[2]);
			sap_data->rain_fall = atof(row[3]);
			sap_data->wind_speed = atof(row[4]);
			//sap_data->wind_direction = atof(row[5]);

			std::cout << "temperature: " << sap_data->temper << " ";
			std::cout << "presure: " << sap_data->air_press << " ";
			std::cout << "humidity: " << sap_data->hum << " ";
			std::cout << "precipitation: " << sap_data->rain_fall << " ";
			std::cout << "wind_speed: " << sap_data->wind_speed << " ";
			//std::cout << "wind_direction: " << sap_data->humidity << std::endl;
		}
		mysql_free_result(res);
	}
	else
	{
		std::cout << "select error：" << mysql_error(&db_g2020) << std::endl;
		if (0 != mysql_ping(&db_g2020))
		{
			mysql_close(&db_g2020);
			init_db();
		}
	}
}