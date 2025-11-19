// server.cpp
#include "server.h"
#include "request_handler.h"

BankServer::BankServer(int p) : port(p), running(false) {
#ifdef _WIN32
    server_fd = INVALID_SOCKET;
#endif
}

BankServer::~BankServer() {
    stop();
}

bool BankServer::start() {
#ifdef _WIN32
    // 初始化Winsock
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup失败: " << result << std::endl;
        return false;
    }

    server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_fd == INVALID_SOCKET) {
        std::cerr << "Socket创建失败: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return false;
    }
#else
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        std::cerr << "Socket创建失败" << std::endl;
        return false;
    }
#endif

    // 设置socket选项
    int opt = 1;
#ifdef _WIN32
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) == SOCKET_ERROR) {
        std::cerr << "设置socket选项失败: " << WSAGetLastError() << std::endl;
        closesocket(server_fd);
        WSACleanup();
        return false;
    }
#else
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        std::cerr << "设置socket选项失败" << std::endl;
        return false;
    }
#endif

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;  // 绑定到所有地址，包括127.0.0.1和::1
    address.sin_port = htons(port);
#ifdef _WIN32
    if (bind(server_fd, (sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        std::cerr << "绑定端口失败: " << WSAGetLastError() << std::endl;
        closesocket(server_fd);
        WSACleanup();
        return false;
    }

    if (listen(server_fd, 10) == SOCKET_ERROR) {
        std::cerr << "监听失败: " << WSAGetLastError() << std::endl;
        closesocket(server_fd);
        WSACleanup();
        return false;
    }
#else
    if (bind(server_fd, (sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "绑定端口失败" << std::endl;
        return false;
    }

    if (listen(server_fd, 10) < 0) {
        std::cerr << "监听失败" << std::endl;
        return false;
    }
#endif

    running = true;
    std::cout << "测试服务器启动成功，端口: " << port << std::endl;
    std::cout << "等待前端连接..." << std::endl;

    // 在新线程中接受连接
    std::thread acceptThread(&BankServer::acceptConnections, this);
    acceptThread.detach();

    return true;
}

void BankServer::stop() {
    running = false;
#ifdef _WIN32
    if (server_fd != INVALID_SOCKET) {
        closesocket(server_fd);
        server_fd = INVALID_SOCKET;
    }
    WSACleanup();
#else
    if (server_fd > 0) {
        close(server_fd);
    }
#endif
}

void BankServer::acceptConnections() {
    while (running) {
#ifdef _WIN32
        SOCKET client_socket = accept(server_fd, nullptr, nullptr);
        if (client_socket == INVALID_SOCKET) {
            if (running) {
                std::cerr << "接受连接失败: " << WSAGetLastError() << std::endl;
            }
            continue;
        }
#else
        int client_socket = accept(server_fd, nullptr, nullptr);
        if (client_socket < 0) {
            if (running) {
                std::cerr << "接受连接失败" << std::endl;
            }
            continue;
        }
#endif

        std::cout << "新的客户端连接" << std::endl;

        std::thread client_thread(&BankServer::handleClient, this, client_socket);
        client_thread.detach();
    }
}

void BankServer::handleClient(SOCKET client_socket) {
    char buffer[4096] = { 0 };

#ifdef _WIN32
    int bytes_read = recv(client_socket, buffer, 4096 - 1, 0);
#else
    ssize_t bytes_read = read(client_socket, buffer, 4096 - 1);
#endif

    if (bytes_read > 0) {
        std::string request(buffer, bytes_read);
        std::string response = RequestHandler::processRequest(request);

        std::cout << "发送响应: " << response << std::endl;

#ifdef _WIN32
        send(client_socket, response.c_str(), response.length(), 0);
#else
        send(client_socket, response.c_str(), response.length(), 0);
#endif
    }
    else {
        std::cout << "客户端断开连接" << std::endl;
    }

#ifdef _WIN32
    closesocket(client_socket);
#else
    close(client_socket);
#endif
}