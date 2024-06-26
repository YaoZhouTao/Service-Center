#include "epoll.h"

EquipmentMessageType* Epoll::Electrical_Equipment_ptr = nullptr;

EquipmentMessageType* Epoll::Mental_Entertainment_Equipment_ptr = nullptr;

EquipmentMessageType* Epoll::Lease_Equipment_ptr = nullptr;

Epoll::Epoll()
{
	Destroy_Epool_Class = false;
	ListenFd = -1;
	ClientFd = -1;
	Epfd = -1;
	this->initListenFd();
	Electrical_Equipment_ptr = new Electrical_Equipment;
	Mental_Entertainment_Equipment_ptr = new Mental_Entertainment_Equipment;
	Lease_Equipment_ptr = new Lease_Equipment;
	Client_Accept = new ThreadPool<AcceptInfo>(20,100,"Client_Accept");
	Recv_Client_Date = new ThreadPool<RecvDataInfo>(20, 100, "Recv_Client_Date");
	ManagerTimeOUT = new thread(Connect_TimeOut_Thread,this);
	//cout << "Epoll初始化完成,监听端口" << this->port << "......" << endl;
	info("Epoll初始化完成,监听端口%s",port);
}

Epoll::~Epoll()
{
	Destroy_Epool_Class = true;
	if (Client_Accept != nullptr) {
		delete Client_Accept;
		Client_Accept = nullptr;
	}
	else if (Recv_Client_Date != nullptr) {
		delete Recv_Client_Date;
		Recv_Client_Date = nullptr;
	}
	else if (Electrical_Equipment_ptr != nullptr && Mental_Entertainment_Equipment_ptr != nullptr && Lease_Equipment_ptr != nullptr) {
		delete Electrical_Equipment_ptr;
		delete Mental_Entertainment_Equipment_ptr;
		delete Lease_Equipment_ptr;
		Electrical_Equipment_ptr = Mental_Entertainment_Equipment_ptr = Lease_Equipment_ptr = nullptr;
	}
	else if (ManagerTimeOUT != nullptr) {
		if (ManagerTimeOUT->joinable()) {
			ManagerTimeOUT->join();
		}
		delete ManagerTimeOUT;
		ManagerTimeOUT = nullptr;
	}
	//cout << "***** Epoll对象已经被销毁 *****" << endl;
	info("***** Epoll对象已经被销毁 *****");
}

int Epoll::epollRun()
{
	//1.创建epoll实列
	int epfd = epoll_create(1);
	if (epfd == -1) {
		perror("epoll_create");
		return -1;
	}

	Epfd = epfd;

	//2.lfd上树
	struct epoll_event ev;
	ev.data.fd = ListenFd;
	ev.events = EPOLLIN;
	int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, ListenFd, &ev);
	if (ret == -1) {
		perror("epoll_ctl");
		return -1;
	}
	//3.检测
	struct epoll_event evs[1024];
	int size = sizeof(evs) / sizeof(struct epoll_event);
	//cout << "EpollRun已启动..." << endl;
	info("EpollRun已启动...");
	while (1) {
		int num = epoll_wait(epfd, evs, size, -1);
		for (int i = 0; i < num; ++i) {
			int fd = evs[i].data.fd;
			if (fd == ListenFd) {
				ClientFd = accept(fd, (struct sockaddr*)&client_addr, &client_addr_len);
				if (ClientFd == -1) {
					perror("accept: ");
				}
				else {
					AcceptInfo* threadarg = new AcceptInfo(ClientFd,client_addr,this);
					if (threadarg == nullptr) {
						error("动态申请内存失败");
					}
					else {
						Client_Accept->addTask(Task<AcceptInfo>(AcceptClient, threadarg));
					}
				}
			}
			else {
				RecvDataInfo* threadarg = new RecvDataInfo{
					.fd = fd,
					.epoll = this
				};
				if (threadarg == nullptr) {
					error("动态申请内存失败");
				}
				else {
					Recv_Client_Date->addTask(Task<RecvDataInfo>(RecvClientDate, threadarg));
				}
			}
		}
	}
	return 0;
}

void Epoll::initListenFd()
{
	//1.创建监听的fd
	int listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd == -1) {
		perror("socket");
		return;
	}

	//设置为监听文件描述符为非阻塞模式
	int flags = fcntl(listenfd, F_GETFL, 0);
	fcntl(listenfd, F_SETFL, flags | O_NONBLOCK);

	//2.设置端口复用
	int opt = 1;
	int ret = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (ret == -1) {
		perror("setsockopt");
		return;
	}

	//3.绑定
	unsigned short port = static_cast<short unsigned int>(atoi(this->port));
	struct sockaddr_in  addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;
	ret = bind(listenfd, (struct sockaddr*)&addr, sizeof(addr));
	if (ret == -1) {
		perror("bind");
		return;
	}

	//4.设置监听
	ret = listen(listenfd, 5000);
	if (ret == -1) {
		perror("listen");
		return;
	}

	//5.返回fd
	this->ListenFd = listenfd;
}

void Epoll::AcceptClient(void* arg)
{
	AcceptInfo* Arg = (AcceptInfo*)(arg);
	//设置非阻塞
	int flag = fcntl(Arg->cfd, F_GETFL);
	flag |= O_NONBLOCK;
	fcntl(Arg->cfd, F_SETFL, flag);

	//cfd添加到epoll
	struct epoll_event ev;
	ev.data.fd = Arg->cfd;
	ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
	Arg->epoll->mutex_epoll_ctl.lock(); // 加锁
	int ret = epoll_ctl(Arg->epoll->Epfd, EPOLL_CTL_ADD, Arg->cfd, &ev);
	Arg->epoll->mutex_epoll_ctl.unlock(); // 解锁
	if (ret == -1) {
		char errorMessage[250];
		strncpy(errorMessage, strerror(errno), 250 - 1);
		errorMessage[250 - 1] = '\0';
		error("Epoll上树失败: %s",errorMessage);
		return;
	}
	Arg->epoll->addClient(Arg->cfd,Arg->client_addr);
	return;
}

void Epoll::RecvClientDate(void* arg)
{
	RecvDataInfo* thread_arg = (RecvDataInfo*)arg;

	char buf[1024]; // 缓冲区 
	memset(buf, 0, sizeof(buf));  // 清空 buffer
	ssize_t ret = recv(thread_arg->fd, buf, 1024, 0); // 接收数据 
	if (ret <= 0) {
		thread_arg->epoll->CloseFd_Epoll_Ctl(thread_arg->fd);
		thread_arg->epoll->removeClient(thread_arg->fd);
		return;
	}

	if (strlen(buf) > 400) {
		error("收到恶意访问数据 %s", buf);
		thread_arg->epoll->CloseFd_Epoll_Ctl(thread_arg->fd);
		thread_arg->epoll->removeClient(thread_arg->fd);
		return;
	}

	//cout << "接收到的数据: " << buf << endl;
	std::string input = string(buf);
	// 任意给定的字符串

	std::regex pattern("TYPE=\\[(\\d+)\\]"); // 匹配[TYPE=xx]，其中xx是一个或多个数字
	std::smatch matches;

	if (std::regex_search(input, matches, pattern)) {
		if (matches.size() >= 2) {
			std::string type_str = matches[1].str(); // 取得第一个捕获组的内容
			std::string type_value = type_str; // 将捕获的字符串保存为字符串类型
			if (type_value == "01") {
				//电气设备报文处理
				Electrical_Equipment_ptr->Message_Type(string(buf));
			}
			else if (type_value == "02") {
				//精神娱乐报文处理
				Mental_Entertainment_Equipment_ptr->Message_Type(string(buf));
			}
			else if (type_value == "03") {
				//租凭设备报文
				Lease_Equipment_ptr->Message_Type(string(buf));
			}
		}
	}
	else {
		//std::cout << "在输入字符串中没有找到TYPE值。" << std::endl;
	}

	char buuuf[100] = { "HTTP1.1 200 OK" };
	ret = send(thread_arg->fd, buuuf, 14, 0);
	thread_arg->epoll->CloseFd_Epoll_Ctl(thread_arg->fd);
	thread_arg->epoll->removeClient(thread_arg->fd);
}

void Epoll::addClient(int client_fd, sockaddr_in client_addr_ip)
{
	client_ip_port_time ip_port_time;
	ip_port_time.client_addr = client_addr_ip;
	time_t current_time = time(NULL);
	ip_port_time.current_time = current_time;
	mutex_client_map.lock();
	client_map[client_fd] = ip_port_time;
	mutex_client_map.unlock();
}

void Epoll::removeClient(int client_fd)
{
	mutex_client_map.lock();
	client_map.erase(client_fd);
	mutex_client_map.unlock();
}

void* Epoll::Connect_TimeOut_Thread(void* arg)
{
	Epoll* epoll = (Epoll*)arg;
	while (epoll->Destroy_Epool_Class == false) {
		epoll->mutex_client_map.lock();
		time_t current_time = time(NULL);
		std::unordered_map<int, client_ip_port_time>::iterator it = epoll->client_map.begin();
		while (it != epoll->client_map.end()) {
			int client_fd = it->first;
			time_t last_active_time = it->second.current_time;
			if (current_time - last_active_time >= TIMEOUT_SECONDS) {
				epoll->mutex_client_map.unlock();
				epoll->mutex_epoll_ctl.lock();
				epoll_ctl(epoll->Epfd, EPOLL_CTL_DEL, client_fd, NULL);
				epoll->mutex_epoll_ctl.unlock();
				close(client_fd);
				epoll->mutex_client_map.lock();
				debug("Closed connection TimeOut: %d", client_fd);
				//cout << "成功删除一个超时客户端" << endl;
				it = epoll->client_map.erase(it);
			}
			else {
				++it;
			}
		}
		epoll->mutex_client_map.unlock();
		sleep(3);
	}
	return NULL;
}

void Epoll::CloseFd_Epoll_Ctl(int cfd)
{
	mutex_epoll_ctl.lock(); // 加锁
	epoll_ctl(Epfd, EPOLL_CTL_DEL, cfd, NULL);
	mutex_epoll_ctl.unlock();  // 解锁
	close(cfd);
}
