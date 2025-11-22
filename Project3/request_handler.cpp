#include "request_handler.h"
#include <iostream>
#include <string>
#include <sstream>
#include"ConnectionPool.h"

std::string RequestHandler::processRequest(const std::string& request_str) {
        //std::cout << " 收到原始请求: " << request_str << std::endl;

        Json::Value root;
        Json::CharReaderBuilder reader;
        std::string errors;
        Json::StreamWriterBuilder writer;
        std::istringstream jsonStream(request_str);
        if (!Json::parseFromStream(reader, jsonStream, &root, &errors)) {
            std::cout << "JSON解析失败: " << errors << std::endl;
        }
        // 1. 首先提取 action
        std::string action = root["action"].asString();
        std::cout << "动作: " << action << std::endl;
        if (action == "user_login")
        {
            return handleUserLogin(request_str);
        }
}


std::string RequestHandler::handleUserLogin(const std::string& Request) {
    Json::Value root;
    Json::CharReaderBuilder reader;
    std::string errors;
   
    std::istringstream jsonStream(Request);
    if (!Json::parseFromStream(reader, jsonStream, &root, &errors)) {
        std::cout << "❌ 登录请求JSON解析失败: " << errors << std::endl;
        return createErrorResponse("登录请求格式错误");
    }
    if (!root.isMember("data") || !root["data"].isObject()) {
        std::cout << "❌ 登录请求缺少data字段" << std::endl;
        return createErrorResponse("登录信息不完整");
    }
    Json::Value data = root["data"];
    std::string username = data["username"].asString();
    std::string password = data["password"].asString();

    std::cout << "登录尝试 - 用户名: " << username << std::endl;
    return ValidLoginInfo(username,password);
}

std::string RequestHandler::ValidLoginInfo(const std::string& username, const std::string& password)
{
    try {
        //JS=角色 Name用户名 Pw=密码
        DatabaseGuard db = DatabaseManager::GetUseInfoCon();
        sql::PreparedStatement* pstmt = db->prepareStatement(
            "SELECT Qdvh_ID, Qdvh_Name, Qdvh_Js FROM userinfo WHERE Qdvh_Name = ? AND Qdvh_Pw = ?"
        );
        pstmt->setString(1, username);
        pstmt->setString(2, password);  // 明文比较
        //std::cout << "compare un,pw" << std::endl;
        sql::ResultSet* res = pstmt->executeQuery();
        std::cout << username << "  " << password << std::endl;

        Json::Value response;
        Json::StreamWriterBuilder writer;

        if (res->next()) {
            // 登录成功
            int userId = res->getInt("Qdvh_ID");
            std::string userName = res->getString("Qdvh_Name");
            std::string role = res->getString("Qdvh_Js");

            std::cout << "登录成功 - 用户ID: " << userId
                << ", 用户名: " << userName
                << ", 角色: " << role << std::endl;

            response["success"] = true;
            response["token"] = "success";
            response["role"] = role;
            response["user"] = userName;
            response["user_id"] = userId;
        }
        else {
            // 登录失败
            std::cout << "❌ 登录失败 - 用户名或密码错误" << std::endl;
            response["success"] = false;
            response["error"] = "用户名或密码错误";
        }

        delete res;
        delete pstmt;
        return Json::writeString(writer, response);

    }
    catch (sql::SQLException& e) {
        std::cout << "💥 数据库错误: " << e.what() << std::endl;
        return createErrorResponse("数据库操作失败");
    }

    
}




























std::string RequestHandler::createErrorResponse(const std::string& error) {
    Json::Value response;
    response["info"] = error;
    Json::StreamWriterBuilder writer;
    return Json::writeString(writer, response);
}
