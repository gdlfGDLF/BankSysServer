#include "request_handler.h"
#include <iostream>
#include <string>
#include <sstream>


std::string RequestHandler::processRequest(const std::string& request_str) {
        //std::cout << "📨 收到原始请求: " << request_str << std::endl;

        Json::Value root;
        Json::CharReaderBuilder reader;
        std::string errors;

        std::istringstream jsonStream(request_str);
        if (!Json::parseFromStream(reader, jsonStream, &root, &errors)) {
            std::cout << "JSON解析失败: " << errors << std::endl;
        }
        // 1. 首先提取 action
        std::string action = root["action"].asString();
        std::cout << "动作: " << action << std::endl;

        // 2. 然后从 data 对象中提取登录信息
        if (root.isMember("data") && root["data"].isObject()) {
            Json::Value data = root["data"];

            std::string username = data["username"].asString();
            std::string password = data["password"].asString();
            std::string loginTime = data["LoginTime"].asString(); // 注意字段名是 "Date"

            std::cout << "用户名: " << username << std::endl;
            std::cout << "密码: " << password << std::endl;
            std::cout << "登录时间: " << loginTime << std::endl;

            return handleUserLogin(username, password);
        }
        else {
            std::cout << "缺少 data 字段或 data 不是对象" << std::endl;

            return createErrorResponse("无法解析登录信息");
        }
}


std::string RequestHandler::handleUserLogin(const std::string& username, const std::string& password) {
    Json::Value response;
    Json::StreamWriterBuilder writer;

    if (username == "admin" && password == "admin123") {
        response["success"] = true;
        response["token"] = "admin_token_123456";
        response["role"] = "admin";
    }
    else if (username == "sales" && password == "sales123") {
        response["success"] = true;
        response["token"] = "sales_token_123456";
        response["role"] = "sales_manager";  // 返回角色信息
    }
    else if (username == "customer" && password == "customer123") {
        response["success"] = true;
        response["token"] = "customer_token_123456";
        response["role"] = "customer_manager";  // 返回角色信息
    }
    else {
        response["success"] = false;
        response["error"] = "用户名或密码错误";
    }

    return Json::writeString(writer, response);
}

//std::string RequestHandler::handleGetUsers() {
//    std::cout << "👥 处理获取用户列表请求" << std::endl;
//
//    std::string usersData = R"("users":[
//        {"id":1,"username":"admin","display_name":"系统管理员","email":"admin@bank.com","role":"admin","is_active":true,"created_at":"2024-01-15 10:00:00"},
//        {"id":2,"username":"zhangsan","display_name":"张三","email":"zhangsan@bank.com","role":"sales_manager","is_active":true,"created_at":"2024-01-16 14:30:00"}
//    ],"total":2,"page":1,"limit":10)";
//
//    return buildJsonResponse(true, "获取用户列表成功", usersData);
//}
//
//std::string RequestHandler::handleAddUser() {
//    std::cout << "➕ 处理添加用户请求" << std::endl;
//
//    std::string responseData = R"("user_id":3,"message":"用户添加成功")";
//    return buildJsonResponse(true, "用户添加成功", responseData);
//}

std::string RequestHandler::createErrorResponse(const std::string& error) {
    Json::Value response;
    response["info"] = error;
    Json::StreamWriterBuilder writer;
    return Json::writeString(writer, response);
}