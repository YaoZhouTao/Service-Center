#ifndef ConnectionPool_H_
#define ConnectionPool_H_
#include<queue>
#include<string>
#include<mutex>
#include<condition_variable>
#include"MysqlConn.h"
#include<memory>
using namespace std;

class ConnectionPool
{
public:
	//通过静态方法获取单列类的对象
	static ConnectionPool* getConnectionPool();
	ConnectionPool(const ConnectionPool& obj) = delete;
	ConnectionPool& operator = (const ConnectionPool& obj) = delete;
	//获取数据库连接
	shared_ptr<MysqlConn> getConnection();
	void close();
private:
	~ConnectionPool();
	ConnectionPool();
	//添加连接的函数
	void addConnection();
	//用于生产数据库连接的任务函数
	void produceConnection();
	//用于销毁数据库连接的任务函数
	void recycleConnection();


	string m_ip = "192.144.215.114";
	string m_user = "root";
	string m_passwd = "Hckj201509";
	string m_dbName = "share1";
	/*string m_ip = "127.0.0.1";
	string m_user = "root";
	string m_passwd = "563128";
	string m_dbName = "share1";*/
	unsigned short m_port = 3306;
	int m_minSize = 100;							//队列中最少有多少个连接
	int m_maxSize = 800;							//队列中最多有多少个连接
	int m_timeout = 1000;							//当队列里没有连接时阻塞，设置超时时间
	int m_maxIdleTime = 10800000;
	//int m_maxIdleTime = 1000;	//设置最大空闲时长，达到最大空闲时长关闭连接
	queue<MysqlConn*> m_connectionQ;		//存放有效连接的队列
	mutex m_mutexQ;
	condition_variable m_cond;
	bool ISClose = true;

	static int busynum;
};

#endif



