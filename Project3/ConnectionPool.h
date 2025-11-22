#pragma once
#pragma once
#include <queue>
#include <mutex>
#include <memory>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h> 
#include <cppconn/resultset.h>
#include <iostream>

class UniversalConnectionPool {
private:
    std::queue<sql::Connection*> connections;
    std::mutex mtx;
    std::string url, username, password;

public:
    UniversalConnectionPool(const std::string& url, const std::string& user, const std::string& pwd);
    ~UniversalConnectionPool();

    sql::Connection* getConnection(const std::string& database = "");
    void returnConnection(sql::Connection* con);
    void cleanup();
    int getAvailableCount();

private:
    void initializePool();
    sql::Connection* createNewConnection();
};

class DatabaseGuard {
private:
    sql::Connection* con;
    UniversalConnectionPool& pool;
    std::string currentDB;

public:
    DatabaseGuard(UniversalConnectionPool& p, const std::string& database = "");
    ~DatabaseGuard();

    sql::Connection* operator->() { return con; }
    sql::Connection* get() { return con; }
    void switchDatabase(const std::string& newDatabase);
};

class DatabaseManager {
private:
    static UniversalConnectionPool& getMainPool();

public:
    static DatabaseGuard GetUseInfoCon();
    static DatabaseGuard getLogDatabaseConnection();
    static DatabaseGuard getReportDatabaseConnection();
    static DatabaseGuard getGenericConnection();

    static void shutdown();  // 可选：程序退出时清理
};