#include "request_handler.h"
#include <iostream>
#include <string>
#include <sstream>

std::string RequestHandler::processRequest(const std::string& request_str) {
    std::cout << "📨 收到原始请求: " << request_str << std::endl;

    // 简单的字符串解析（临时方案）
    if (request_str.find("\"action\":\"user_login\"") != std::string::npos) {
       
        // 提取用户名和密码
        size_t user_pos = request_str.find("\"username\":\"");
        size_t pass_pos = request_str.find("\"password\":\"");
        if (user_pos != std::string::npos && pass_pos != std::string::npos) {
            user_pos += 12; // "\"username\":\"".length()
            pass_pos += 12; // "\"password\":\"".length()

            size_t user_end = request_str.find("\"", user_pos);
            size_t pass_end = request_str.find("\"", pass_pos);

            if (user_end != std::string::npos && pass_end != std::string::npos) {
                std::string username = request_str.substr(user_pos, user_end - user_pos);
                std::string password = request_str.substr(pass_pos, pass_end - pass_pos);
                std::cout << username << "" << password << std::endl;
                return handleUserLogin(username, password);
            }
        }
        return createErrorResponse("无法解析登录信息");
    }
    else if (request_str.find("\"action\":\"get_users\"") != std::string::npos) {
        return handleGetUsers();
    }
    else if (request_str.find("\"action\":\"add_user\"") != std::string::npos) {
        return handleAddUser();
    }
    else if (request_str.find("\"action\":\"test_connection\"") != std::string::npos) {
        return handleTestConnection();
    }
    else {
        return createErrorResponse("未知的请求类型");
    }
}

std::string RequestHandler::handleTestConnection() {
    return buildJsonResponse(true, "C++服务器连接正常", R"("server":"Bank CRM System","timestamp":"2024-01-15 10:00:00")");
}

std::string RequestHandler::handleUserLogin(const std::string& username, const std::string& password) {
    std::cout << "🔐 处理登录 - 用户名: " << username << ", 密码: " << password << std::endl;

    if (username == "admin" && password == "admin123") {
        std::string userData = R"("token":"admin_token_123456","user":{"id":1,"username":"admin","display_name":"系统管理员","role":"admin","email":"admin@bank.com"})";
        return buildJsonResponse(true, "登录成功", userData);
    }
    else {
        return createErrorResponse("用户名或密码错误");
    }
}

std::string RequestHandler::handleGetUsers() {
    std::cout << "👥 处理获取用户列表请求" << std::endl;

    std::string usersData = R"("users":[
        {"id":1,"username":"admin","display_name":"系统管理员","email":"admin@bank.com","role":"admin","is_active":true,"created_at":"2024-01-15 10:00:00"},
        {"id":2,"username":"zhangsan","display_name":"张三","email":"zhangsan@bank.com","role":"sales_manager","is_active":true,"created_at":"2024-01-16 14:30:00"}
    ],"total":2,"page":1,"limit":10)";

    return buildJsonResponse(true, "获取用户列表成功", usersData);
}

std::string RequestHandler::handleAddUser() {
    std::cout << "➕ 处理添加用户请求" << std::endl;

    std::string responseData = R"("user_id":3,"message":"用户添加成功")";
    return buildJsonResponse(true, "用户添加成功", responseData);
}

std::string RequestHandler::createErrorResponse(const std::string& error) {
    return buildJsonResponse(false, error);
}

std::string RequestHandler::buildJsonResponse(bool success, const std::string& message, const std::string& data) {
    std::stringstream json;
    json << "{";
    json << "\"success\":" << (success ? "true" : "false") << ",";

    if (success) {
        json << "\"message\":\"" << message << "\"";
    }
    else {
        json << "\"error\":\"" << message << "\"";
    }

    if (!data.empty()) {
        json << "," << data;
    }

    json << "}";
    return json.str();
}