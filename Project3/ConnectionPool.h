#pragma once
#include <queue>
#include <mutex>
#include <memory>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/statement.h>
#include <cppconn/resultset.h>

class SimpleConnectionPool {
private:
    std::queue<sql::Connection*> connections;
    std::mutex mtx;
    std::string url, username, password, schema;

public:
    SimpleConnectionPool(const std::string& url, const std::string& user,
        const std::string& pwd, const std::string& db)
        : url(url), username(user), password(pwd), schema(db) {
        initializePool();
    }

    ~SimpleConnectionPool() {
        while (!connections.empty()) {
            sql::Connection* con = connections.front();
            connections.pop();
            if (con) {
                con->close();
                delete con;
            }
        }
    }

private:
    void initializePool() {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();

        // 创建3个初始连接
        for (int i = 0; i < 3; ++i) {
            sql::Connection* con = driver->connect(url, username, password);
            con->setSchema(schema);
            connections.push(con);
        }
        std::cout << "✅ 连接池初始化完成" << std::endl;
    }

public:
    sql::Connection* getConnection() {
        std::lock_guard<std::mutex> lock(mtx);

        if (connections.empty()) {
            // 池为空时创建新连接
            sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
            sql::Connection* con = driver->connect(url, username, password);
            con->setSchema(schema);
            return con;
        }

        sql::Connection* con = connections.front();
        connections.pop();
        return con;
    }

    void returnConnection(sql::Connection* con) {
        std::lock_guard<std::mutex> lock(mtx);
        if (con && !con->isClosed()) {
            connections.push(con);
        }
    }
};

class ConnectionGuard {
private:
    sql::Connection* con;
    SimpleConnectionPool& pool;

public:
    ConnectionGuard(SimpleConnectionPool& p) : pool(p) {
        con = pool.getConnection();
        std::cout << "🔗 获取数据库连接" << std::endl;
    }

    ~ConnectionGuard() {
        pool.returnConnection(con);
        std::cout << "↩️  归还数据库连接" << std::endl;
    }

    sql::Connection* operator->() { return con; }
    sql::Connection* get() { return con; }
};

// 全局连接池实例
SimpleConnectionPool& getConnectionPool() {
    static SimpleConnectionPool pool(
        "tcp://127.0.0.1:3306",
        "root",
        "your_password",
        "bank_crm"
    );
    return pool;
}