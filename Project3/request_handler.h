#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include <string>
#include<json.h>

class RequestHandler {
public:
    static std::string processRequest(const std::string& request_str);

private:
    static std::string handleUserLogin(const std::string& Request);
    //static std::string handleGetUsers();
    //static std::string handleAddUser();
    static std::string createErrorResponse(const std::string& error);
    static std::string ValidLoginInfo(const std::string& username, const std::string& password);
    // 简单的JSON构建函数
    static std::string buildJsonResponse(bool success, const std::string& message, const Json::Value& data);
};

#endif