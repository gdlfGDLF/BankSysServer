#include "ConnectionPool.h"
// UniversalConnectionPool 实现
UniversalConnectionPool::UniversalConnectionPool(const std::string& url,
    const std::string& user,
    const std::string& pwd)
    : url(url), username(user), password(pwd) {
    initializePool();
}

UniversalConnectionPool::~UniversalConnectionPool() {
    cleanup();
}

void UniversalConnectionPool::initializePool() {
    for (int i = 0; i < 3; ++i) {
        sql::Connection* con = createNewConnection();
        if (con) {
            connections.push(con);
        }
    }
    std::cout << "连接池初始化完成，初始连接数: " << connections.size() << std::endl;
}

sql::Connection* UniversalConnectionPool::createNewConnection() {
    try {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        sql::Connection* con = driver->connect(url, username, password);
        std::cout << "创建新数据库连接" << std::endl;
        return con;
    }
    catch (sql::SQLException& e) {
        std::cout << "创建连接失败: " << e.what() << std::endl;
        return nullptr;
    }
}

sql::Connection* UniversalConnectionPool::getConnection(const std::string& database) {
    std::lock_guard<std::mutex> lock(mtx);

    sql::Connection* con = nullptr;

    if (connections.empty()) {
        std::cout << "连接池已空，创建新连接" << std::endl;
        con = createNewConnection();
    }
    else {
        con = connections.front();
        connections.pop();
    }

    // 如果指定了数据库，切换到该数据库
    if (con && !con->isClosed() && !database.empty()) {
        try {
            con->setSchema(database);
        }
        catch (sql::SQLException& e) {
            std::cout << "切换数据库失败: " << e.what() << std::endl;
        }
    }

    return con;
}

void UniversalConnectionPool::returnConnection(sql::Connection* con) {
    std::lock_guard<std::mutex> lock(mtx);

    if (con && !con->isClosed()) {
        connections.push(con);
    }
    else if (con) {
        // 连接已关闭，直接删除
        delete con;
    }
}

void UniversalConnectionPool::cleanup() {
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << "🧹 清理连接池，连接数: " << connections.size() << std::endl;

    while (!connections.empty()) {
        sql::Connection* con = connections.front();
        connections.pop();
        if (con) {
            con->close();
            delete con;
        }
    }
}

int UniversalConnectionPool::getAvailableCount() {
    std::lock_guard<std::mutex> lock(mtx);
    return connections.size();
}

// DatabaseGuard 实现
DatabaseGuard::DatabaseGuard(UniversalConnectionPool& p, const std::string& database)
    : pool(p), currentDB(database) {
    con = pool.getConnection(database);
    if (con && !database.empty()) {
        std::cout << "获取数据库连接: " << database << std::endl;
    }
}

DatabaseGuard::~DatabaseGuard() {
    pool.returnConnection(con);
    if (!currentDB.empty()) {
        std::cout << "归还数据库连接: " << currentDB << std::endl;
    }
}

void DatabaseGuard::switchDatabase(const std::string& newDatabase) {
    if (con && !con->isClosed()) {
        try {
            con->setSchema(newDatabase);
            currentDB = newDatabase;
            std::cout << "切换到数据库: " << newDatabase << std::endl;
        }
        catch (sql::SQLException& e) {
            std::cout << "切换数据库失败: " << e.what() << std::endl;
        }
    }
}

// DatabaseManager 实现
UniversalConnectionPool& DatabaseManager::getMainPool() {
    static UniversalConnectionPool pool(
        "tcp://127.0.0.1:3306",
        "root",
        "123456"  // 请修改为您的密码
    );
    std::cout << "success link to db"<<std::endl;
    return pool;
}

DatabaseGuard DatabaseManager::GetUseInfoCon() {
    return DatabaseGuard(getMainPool(), "banksys");
}

DatabaseGuard DatabaseManager::getLogDatabaseConnection() {
    return DatabaseGuard(getMainPool(), "system_logs");
}

DatabaseGuard DatabaseManager::getReportDatabaseConnection() {
    return DatabaseGuard(getMainPool(), "business_reports");
}

DatabaseGuard DatabaseManager::getGenericConnection() {
    return DatabaseGuard(getMainPool());
}

void DatabaseManager::shutdown() {
    // 清理连接池
    getMainPool().cleanup();
}