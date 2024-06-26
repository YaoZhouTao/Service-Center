#include "MessageType.h"
/*          电器设备的功率采集报文      
********************************************/

void Electrical_Equipment::Message_Type(string data)
{
	//cout << "电气设备: " << data << endl;

    std::regex pattern("SN=\\[([^\\]]+)\\]TYPE=\\[([^\\]]+)\\]POWER=\\[([^\\]]+)\\]");
    std::smatch matches;

    if (std::regex_search(data, matches, pattern)) {
        if (matches.size() >= 4) {
            mutex_data.lock();
            sn_value = matches[1].str();
            type_value = matches[2].str();
            power_value = matches[3].str();
            mutex_data.unlock();

            if (sn_value != "" && power_value != "") {
                Insert_MySQL();
            }
        }
    }
    else {
        //std::cout << "No match found in the input string." << std::endl;
        warn("收到无法处理的字符串: %s",data.c_str());
    }
}

void Electrical_Equipment::Insert_MySQL()
{
    time_t NOW = time(NULL);
    struct tm* tid = std::localtime(&NOW);
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tid);
    shared_ptr<MysqlConn> conn = MYSQLpool->getConnection();
    if (conn != NULL) {
        char sql[500];
        mutex_data.lock();
        sprintf(sql, "insert into new_aged_equment_electrical_data(dt,sn,power) values('%s','%s','%s')", buffer, sn_value.c_str(), power_value.c_str());
        mutex_data.unlock();
        conn->update(sql);
    }
    else {
        error("数据库连接池异常，数据库连接已消耗完，无可用连接");
    }
}
/********************************************/



/*           精神娱乐设备的状态报文
-------------------------------------------*/

void Mental_Entertainment_Equipment::Message_Type(string data)
{
	//cout << "精神娱乐设备: " << data << endl;

    std::regex pattern("SN=\\[([^\\]]+)\\]TYPE=\\[([^\\]]+)\\]STATE=\\[([^\\]]+)\\]");
    std::smatch matches;

    if (std::regex_search(data, matches, pattern)) {
        if (matches.size() >= 4) {
            mutex_data.lock();
            sn_value = matches[1].str();
            type_value = matches[2].str();
            state_value = matches[3].str();
            mutex_data.unlock();
            if (sn_value != "" && state_value != "") {
                Insert_MySQL();
            }
        }
    }
    else {
        //std::cout << "No match found in the input string." << std::endl;
        warn("收到无法处理的字符串: %s", data.c_str());
    }
}

void Mental_Entertainment_Equipment::Insert_MySQL()
{
    time_t NOW = time(NULL);
    struct tm* tid = std::localtime(&NOW);
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tid);
    shared_ptr<MysqlConn> conn = MYSQLpool->getConnection();
    if (conn != NULL) {
        char sql[500];
        mutex_data.lock();
        sprintf(sql, "insert into new_aged_equment_entertainment_data(sn,dt,dt_type) values('%s','%s','%s')", sn_value.c_str(), buffer, state_value.c_str());
        mutex_data.unlock();
        conn->update(sql);
    }
    else {
        error("数据库连接池异常，数据库连接已消耗完，无可用连接");
    }
    
}

/*------------------------------------------*/



/*            租赁设备的状态报文
############################################*/

void Lease_Equipment::Message_Type(string data)
{
	//cout << "租凭设备: " << data << endl;

    std::regex pattern("SN=\\[([^\\]]+)\\]TYPE=\\[([^\\]]+)\\]STATE=\\[([^\\]]+)\\]");
    std::smatch matches;

    if (std::regex_search(data, matches, pattern)) {
        if (matches.size() >= 4) {
            mutex_data.lock();
            sn_value = matches[1].str();
            type_value = matches[2].str();
            state_value = matches[3].str();
            mutex_data.unlock();
            if (sn_value != "" && state_value != "") {
                Insert_MySQL();
            }
        }
    }
    else {
        //std::cout << "No match found in the input string." << std::endl;
        warn("收到无法处理的字符串: %s", data.c_str());
    }
}

void Lease_Equipment::Insert_MySQL()
{
    time_t NOW = time(NULL);
    struct tm* tid = std::localtime(&NOW);
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tid);
    shared_ptr<MysqlConn> conn = MYSQLpool->getConnection();
    if (conn != NULL) {
        char sql[500];
        mutex_data.lock();
        sprintf(sql, "insert into new_aged_equment_lease_data(sn,dt,dt_type) values('%s','%s','%s')", sn_value.c_str(), buffer, state_value.c_str());
        mutex_data.unlock();
        conn->update(sql);
    }
    else {
        error("数据库连接池异常，数据库连接已消耗完，无可用连接");
    }
}

/*###########################################*/


