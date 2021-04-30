//
// Created by dellv on 07.03.2021.
//

#ifndef DELLVINX_SERVER_H
#define DELLVINX_SERVER_H
#include <cstdint>
#include <iostream>
#include <vector>
#include <thread>

struct response{
    std::string type;
    uint32_t len;
    std::string description;
    std::string file;
};


class Server {
public:
    Server();

    explicit Server(uint32_t port);

    ~Server();

    int start();

private:
    [[noreturn]] int mainLoop();

    static void *handleClient(void* x);

    static int checkMethod(std::string& req);

    static void setHeaders(std::string& resp, uint32_t contentLen, std::string& contentType);

    static response getResp(std::string& req);

    static std::string getPath(std::string& req);

    static std::string getType(std::string& path);

    static std::string getContent(std::string &path, std::string &read);

    void error(const char *msg);

private:
    uint32_t _port;
    int listen_socket{};
};


#endif //DELLVINX_SERVER_H
