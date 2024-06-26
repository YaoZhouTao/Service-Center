#ifndef _LOGGER_H
#define _LOGGER_H

#include <string>
#include <fstream>
#include<mutex>
using namespace std;


#define debug(format, ...) \
    Logger::instance()->log(Logger::DEBUG, __FILE__, __LINE__, format, ##__VA_ARGS__)

#define info(format, ...) \
    Logger::instance()->log(Logger::INFO, __FILE__, __LINE__, format, ##__VA_ARGS__)

#define warn(format, ...) \
    Logger::instance()->log(Logger::WARN, __FILE__, __LINE__, format, ##__VA_ARGS__)

#define error(format, ...) \
    Logger::instance()->log(Logger::ERROR, __FILE__, __LINE__, format, ##__VA_ARGS__)

#define fatal(format, ...) \
    Logger::instance()->log(Logger::FATAL, __FILE__, __LINE__, format, ##__VA_ARGS__)


class Logger
{
public:
    enum Level
    {
        DEBUG = 0,
        INFO,
        WARN,
        ERROR,
        FATAL,
        LEVEL_COUNT
    };

    static Logger* instance();
    void open(const string& filename);
    void log(Level level, const char* file, int line, const char* format, ...);
    void max(int bytes);
    void level(int level);
    void TimerThread();
private:
    Logger();
    ~Logger();
    // 添加细粒度锁成员变量
    static mutex m_instanceMutex;
    static mutex m_fileMutex;

private:
    int flag = 0;
    string m_filename;
    ofstream m_fout;
    int m_max;
    int m_len;
    int m_level;
    static const char* s_level[LEVEL_COUNT];
    static Logger* m_instance;
};


#endif