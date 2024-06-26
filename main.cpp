#include <cstdio>
#include"epoll.h"

int main()
{
    // 初始化日志库
    string filename = "";
    time_t NOW = time(NULL);
    struct tm* t = std::localtime(&NOW);
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S.txt", t);
    filename = buffer;
    // 初始化日志对象
    Logger::instance()->open(filename);
    //初始化并启动epoll
    Epoll* epoll = new Epoll;
    epoll->epollRun();
    delete epoll;
    return 0;
}

