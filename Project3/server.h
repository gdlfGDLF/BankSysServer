// server.h
#ifndef SERVER_H
#define SERVER_H

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

#include <thread>
#include <memory>
#include <string>
#include <iostream>

class BankServer {
private:
#ifdef _WIN32
    SOCKET server_fd;
    WSADATA wsaData;
#else
    int server_fd;
#endif
    int port;
    bool running;

public:
    BankServer(int port);
    ~BankServer();

    bool start();
    void stop();
    bool isRunning() const { return running; }

private:
    void acceptConnections();
    void handleClient(SOCKET client_socket);
};

#endif