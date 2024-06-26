#include "Logger.h"
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <stdexcept>
#include <thread>
#include <iostream>

std::mutex Logger::m_instanceMutex;
std::mutex Logger::m_fileMutex;


const char* Logger::s_level[LEVEL_COUNT] =
{
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR",
    "FATAL"
};

Logger* Logger::m_instance = NULL;

Logger::Logger() : m_max(0), m_len(0), m_level(DEBUG)
{
    std::thread timer_thread(&Logger::TimerThread, this);
    timer_thread.detach();
}

Logger::~Logger()
{
    m_fout.close();
    free(m_instance);
}

Logger* Logger::instance()
{
    // 使用细粒度锁保护实例创建
    std::lock_guard<std::mutex> lock(m_instanceMutex);
    if (m_instance == NULL) {
        m_instance = new Logger();
    }
    return m_instance;
}

void Logger::open(const string& filename)
{
    // 使用细粒度锁保护文件打开操作
    std::lock_guard<std::mutex> lock(m_fileMutex);
    if (flag == 1) {
        m_fout.close();
        flag = 0;
    }
    m_filename = filename;
    m_fout.open(filename, ios::app);
    if (m_fout.fail())
    {
        throw std::logic_error("open log file failed: " + filename);
    }
    m_fout.seekp(0, ios::end);
    m_len = m_fout.tellp();
}

void Logger::log(Level level, const char* file, int line, const char* format, ...)
{
    if (m_level > level)
    {
        return;
    }

    /* if (m_fout.fail())
     {
         throw std::logic_error("open log file failed: " + m_filename);
     }*/

    time_t ticks = time(NULL);
    struct tm* ptm = localtime(&ticks);
    char timestamp[32];
    memset(timestamp, 0, sizeof(timestamp));
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", ptm);

    int len = 0;
    const char* fmt = "%s %s %s:%d ";
    // 使用细粒度锁保护文件关闭操作
    std::lock_guard<std::mutex> lock(m_fileMutex);
    len = snprintf(NULL, 0, fmt, timestamp, s_level[level], file, line);
    if (len > 0)
    {
        char* buffer = new char[len + 1];
        snprintf(buffer, len + 1, fmt, timestamp, s_level[level], file, line);
        buffer[len] = 0;
        m_fout << buffer;
        delete buffer;
        buffer = nullptr;
        m_len += len;
    }

    va_list arg_ptr;
    va_start(arg_ptr, format);
    len = vsnprintf(NULL, 0, format, arg_ptr);
    va_end(arg_ptr);
    if (len > 0)
    {
        char* content = new char[len + 1];
        va_start(arg_ptr, format);
        vsnprintf(content, len + 1, format, arg_ptr);
        va_end(arg_ptr);
        content[len] = 0;
        m_fout << content;
        delete content;
        m_len += len;
    }

    m_fout << "\n";
    m_fout.flush();

}

void Logger::max(int bytes)
{
    m_max = bytes;
}

void Logger::level(int level)
{
    m_level = level;
}

void Logger::TimerThread()
{
    while (true)
    {
        //获取当前时间
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        std::tm* t = std::localtime(&now_time);

        // 判断当前时间是否已经超过了凌晨12点
        if (t->tm_hour >= 0 && t->tm_min >= 0 && t->tm_sec >= 0) {
            // 当前时间已经超过了凌晨12点，将下一个凌晨12点的时间点增加一天
            t->tm_mday += 1;
        }

        // 将时间调整为凌晨12点
        t->tm_hour = 0;
        t->tm_min = 0;
        t->tm_sec = 0;

        // 计算距离下一个凌晨12点的时间间隔
        std::time_t next_midnight_time = std::mktime(t);
        std::chrono::system_clock::time_point next_midnight_point = std::chrono::system_clock::from_time_t(next_midnight_time);
        std::chrono::seconds sleep_duration = std::chrono::duration_cast<std::chrono::seconds>(next_midnight_point - now);

        std::chrono::seconds duration_seconds = std::chrono::duration_cast<std::chrono::seconds>(sleep_duration);
        //std::cout << "Sleep duration: " << duration_seconds.count() << " seconds" << std::endl;
        // 等待到凌晨12点
        std::this_thread::sleep_for(sleep_duration);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        flag = 1;

        //std::this_thread::sleep_for(std::chrono::milliseconds(10000));

        // 生成新的文件名
        time_t NOW = time(NULL);
        struct tm* tid = std::localtime(&NOW);
        char buffer[80];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S.txt", tid);
        m_filename = buffer;
        // 关闭和重新打开文件
        open(m_filename);
    }
}
