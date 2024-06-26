#include "ConnectionPool.h"
#include <thread>
#include <pthread.h>
#include "Logger.h"

int ConnectionPool::busynum = 0;

ConnectionPool* ConnectionPool::getConnectionPool()
{
    static ConnectionPool pool;
    return &pool;
}

shared_ptr<MysqlConn> ConnectionPool::getConnection()
{
    unique_lock<mutex> locker(m_mutexQ);
    while (m_connectionQ.empty()) {
        //error("MYSQL连接池为空，无可用连接");
        if (cv_status::timeout == m_cond.wait_for(locker, chrono::milliseconds(m_timeout))) {
            if (m_connectionQ.empty()) {
                continue;
            }
            else {
                break;
            }
        }
    }
    shared_ptr<MysqlConn> connptr(m_connectionQ.front(), [this](MysqlConn* conn) {
        lock_guard<mutex> locker(m_mutexQ);
        conn->refresAliveTime();
        m_connectionQ.push(conn);
        busynum--;
        });
    m_connectionQ.pop();
    busynum++;
    m_cond.notify_all();
    return connptr;
}

void ConnectionPool::close()
{
    ISClose = false;
    m_cond.notify_all();
}

ConnectionPool::~ConnectionPool()
{

    while (!m_connectionQ.empty()) {
        MysqlConn* conn = m_connectionQ.front();
        m_connectionQ.pop();
        delete conn;
        conn = NULL;
    }
}

ConnectionPool::ConnectionPool()
{
    for (int i = 0; i < m_minSize; ++i) {
        addConnection();
    }
    //thread producer(&ConnectionPool::produceConnection, this);
    thread recycler(&ConnectionPool::recycleConnection, this);
    //producer.detach();
    recycler.detach();
}

void ConnectionPool::addConnection()
{
    MysqlConn* conn = new MysqlConn;
    bool a = conn->connect(m_user, m_passwd, m_dbName, m_ip, m_port);
    if (a == true) {
        conn->refresAliveTime();
        m_connectionQ.push(conn);
        //cout << "成功添加一个连接" << endl;
    }
    else {
        cout << "添加一个连接失败" << endl;
        warn("连接数据库失败");
    }
}

void ConnectionPool::produceConnection()
{
    while (ISClose) {
        unique_lock<mutex> locker(m_mutexQ);
        while (int(m_connectionQ.size()) >= m_minSize) {
            m_cond.wait(locker);
            if (ISClose == false) {
                cout << "produceConnection()退出..." << endl;
                return;
            }
        }
        if (((m_connectionQ.size() + busynum) < m_maxSize && m_connectionQ.size() < m_minSize)) {
            addConnection();
        }
        else if ((m_connectionQ.size() + busynum) == m_maxSize) {
            cout << "已经达到最大连接数所有连接都在忙碌 busy = " << busynum << endl;
        }
        m_cond.notify_all();
    }
}

void ConnectionPool::recycleConnection()
{
    while (ISClose) {
        this_thread::sleep_for(chrono::seconds(3600)); //延迟0.5秒
        lock_guard<mutex> locker(m_mutexQ);
        if (!m_connectionQ.empty()) {
            MysqlConn* conn = m_connectionQ.front();
            long long livetime = conn->getAliveTime();
            //cout << "一个连接的空闲时间 = " << livetime << endl;
            if (conn->getAliveTime() >= m_maxIdleTime) {
                m_connectionQ.pop();
                delete conn;
                conn = nullptr;
                cout << "删除一个超时连接" << endl;
                warn("删除一个超时连接超时");
                addConnection();
            }
            /*if (m_connectionQ.size() > busynum && m_connectionQ.size() > m_minSize) {
                MysqlConn* conn = m_connectionQ.front();
                m_connectionQ.pop();
                if (conn != NULL) {
                    delete conn;
                    conn = nullptr;
                    cout << "删除一个多余连接,当前连接剩余数 = " << m_connectionQ.size() << endl;
                }
            }*/
            conn = nullptr;
        }


    }
    cout << "recycleConnection()退出..." << endl;
}
