#ifndef _MESSAGETYPE_H_
#define _MESSAGETYPE_H_
#include<iostream>
#include<string>
#include <regex>
#include"ConnectionPool.h"
#include"MysqlConn.h"
#include "Logger.h"
using namespace std;
//设备报文类型父类
class EquipmentMessageType
{
public:
	virtual void Message_Type(string data) = 0;
	virtual ~EquipmentMessageType(){}
public:
	ConnectionPool* MYSQLpool = ConnectionPool::getConnectionPool();
};

//电器设备的功率采集报文
class Electrical_Equipment : public EquipmentMessageType {
public:
	void Message_Type(string data) override;

private:
	void Insert_MySQL();
	mutex mutex_data;

	std::string sn_value;
	std::string type_value;
	std::string power_value;
};

//精神娱乐设备的状态报文
class Mental_Entertainment_Equipment : public EquipmentMessageType {
public:
	void Message_Type(string data) override;

private:
	void Insert_MySQL();
	mutex mutex_data;

	std::string sn_value;
	std::string type_value;
	std::string state_value;
};

//租赁设备的状态报文
class Lease_Equipment : public EquipmentMessageType {
public:
	void Message_Type(string data) override;

private:
	void Insert_MySQL();
	mutex mutex_data;

	std::string sn_value;
	std::string type_value;
	std::string state_value;
};


#endif

