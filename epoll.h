#ifndef _EPOLL_H_
#define _EPOLL_H_

#include<iostream>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <queue>
#include <thread>
#include <unordered_map>
#include <regex>
#include<mutex>
#include "ThreadPool.hpp"
#include "TaskQueue.hpp"
#include "MessageType.h"

#define TIMEOUT_SECONDS 15  //单位秒

using namespace std;

class Epoll
{
public:
	Epoll();
	~Epoll();
	// 启动epoll
	int epollRun();

private:
	//初始化监听的套接字
	void initListenFd();
	//客户端fd上树
	static void  AcceptClient(void* arg);
	//接收客户端数据
	static void RecvClientDate(void* arg);
	// 添加客户端连接和时间戳到哈希表中
	void addClient(int client_fd, struct sockaddr_in client_addr_ip);
	// 从哈希表中删除客户端连接和时间戳
	void removeClient(int client_fd);
	// 线程函数，用于检测超时连接并进行处理
	static void* Connect_TimeOut_Thread(void* arg);
	//关闭文件描述符以及下数
	void CloseFd_Epoll_Ctl(int cfd);

private:
	int ListenFd;
	int ClientFd;
	int Epfd;
	const char* port = "20045";
	bool Destroy_Epool_Class;

private:
	//客户端上树需要的变量
	struct AcceptInfo {
		int cfd;
		struct sockaddr_in client_addr;
		Epoll* epoll;

		AcceptInfo(int cfd_, sockaddr_in client_addr_,Epoll* _epoll_)
		: cfd(cfd_), client_addr(client_addr_), epoll(_epoll_)
		{
		}
	};

	struct RecvDataInfo {
		int fd;
		Epoll* epoll;
	};

	struct client_ip_port_time {
		struct sockaddr_in client_addr;
		time_t current_time;
	};

private:
	//客户端IP地址信息
	struct sockaddr_in client_addr;

	//客户端IP地址信息长度
	socklen_t client_addr_len = sizeof(client_addr);

	//锁epoll_ctl树
	mutex mutex_epoll_ctl;

	//锁client_map哈希表
	mutex mutex_client_map;

	//创建一个线程池，处理客户端epoll上树
	ThreadPool<AcceptInfo>* Client_Accept;

	//创建一个线程池，接收数据
	ThreadPool<RecvDataInfo>* Recv_Client_Date;

	//定义一个超时管理线程指针，管理超时连接的客户端
	thread* ManagerTimeOUT;

	// 哈希表用于存储客户端文件描述符和时间戳
	std::unordered_map<int, client_ip_port_time> client_map;

	static EquipmentMessageType* Electrical_Equipment_ptr;

	static EquipmentMessageType* Mental_Entertainment_Equipment_ptr;

	static EquipmentMessageType* Lease_Equipment_ptr;
};

#endif
