// main.cpp
#include "server.h"
#include <iostream>
#include <csignal>

BankServer* serverPtr = nullptr;

// Windows信号处理
BOOL WINAPI ConsoleHandler(DWORD signal) {
    if (signal == CTRL_C_EVENT) {
        std::cout << "\n收到停止信号，正在关闭服务器..." << std::endl;
        if (serverPtr) {
            serverPtr->stop();
        }
        return TRUE;
    }
    return FALSE;
}

int main() {
    // 设置控制台处理程序
    if (!SetConsoleCtrlHandler(ConsoleHandler, TRUE)) {
        std::cout << "无法设置控制台处理程序" << std::endl;
    }

    BankServer server(8080);
    serverPtr = &server;

    std::cout << "=== 银行CRM系统测试服务器 (Windows版本) ===" << std::endl;
    std::cout << "服务器端口: 8080" << std::endl;
    std::cout << "支持的操作:" << std::endl;
    std::cout << "- 用户登录 (admin/admin123)" << std::endl;
    std::cout << "- 获取用户列表" << std::endl;
    std::cout << "- 添加用户" << std::endl;
    std::cout << "按 Ctrl+C 停止服务器" << std::endl;
    std::cout << "===========================================" << std::endl;

    if (server.start()) {
        // 等待服务器停止
        while (server.isRunning()) {
            Sleep(1000); // Windows的sleep
        }
        std::cout << "服务器已停止" << std::endl;
    }
    else {
        std::cerr << "服务器启动失败" << std::endl;
        return 1;
    }

    return 0;
}