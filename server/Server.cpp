//
// Created by dellv on 07.03.2021.
//

#include "Server.h"
#include "../conf/config.h"

#include <sstream>
#include <ctime>
#include <fstream>
#include <algorithm>
#include <filesystem>
#include <unistd.h>
#include <signal.h>

Server::Server() {
    _port = DEFAULT_PORT;
}

Server::Server(uint32_t port) {
    _port = port;
}

int Server::start() {
    signal(SIGPIPE,SIG_IGN);
    struct sockaddr_in server{};
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(_port);
    server.sin_family = AF_INET;
    listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_socket == -1) {
        std::cerr << "Error creating socket:" << listen_socket << std::endl;
        return -1;
    }
    if (bind(listen_socket, (struct sockaddr *) &server, sizeof(server)) < 0) {
        std::cerr << "Error binding:" << listen_socket << std::endl;
        return -1;
    }
    if (listen(listen_socket, SOMAXCONN) < 0) {
        std::cerr << "Error listening:" << listen_socket << std::endl;
        return -1;
    }
    int err = mainLoop();
    if (err) {
        std::cerr << "Error while handling:" << err << std::endl;
    }
    return 0;
}

[[noreturn]] int Server::mainLoop() {
    while (true) {
        int *client_socket=(int *)malloc(sizeof(int));
        *client_socket = accept(listen_socket, nullptr, nullptr);
        if (*client_socket == -1) {
            std::cerr << "accept failed: " << *client_socket << "\n";
            continue;
        }
        pthread_t rec;
        pthread_create(&rec, nullptr, Server::handleClient, (void *) client_socket);
    }
}

Server::~Server() {
    close(listen_socket);
}

void *Server::handleClient(void *x) {
    int client_socket = *(int *) x;
    std::string buf;
    buf.resize(BUFF_SIZE);
    std::string resp = "HTTP/1.1 ";
    std::string contentType;
    int64_t err = recv(client_socket, (char *) buf.c_str(), BUFF_SIZE, 0);
    if (err == -1) {
        std::cerr << client_socket;
        free (x);
        close(client_socket);
        pthread_exit(0);
    }
    if (checkMethod(buf)) {
        resp += "405 Method Not Allowed\r\n";
        setHeaders(resp, 0, contentType);
    } else {
        response resp_struct = getResp(buf);
        resp += resp_struct.description;
        setHeaders(resp, resp_struct.len, resp_struct.type);
        if (buf.find("GET") == 0) {
            resp += resp_struct.file;
        }
    }
//    std::cout<<resp<<std::endl;
    send(client_socket, resp.c_str(), resp.size(), 0);
    err = close(client_socket);
    free (x);
    pthread_exit(0);
}

int Server::checkMethod(std::string &req) {
    if (req.find("GET") == -1 && req.find("HEAD")) {
        return 1;
    }
    return 0;
}

void Server::setHeaders(std::string &resp, uint32_t contentLen, std::string &contentType) {
    time_t now = time(nullptr);
    resp += "Connection: close\r\n";
    resp += "Server: DellvinX (*unix)\r\n";
    std::time_t t = std::time(nullptr);
    resp += "Date: " + std::string(ctime(&now));
    if (!contentType.empty()) {
        resp += "Content-Type: " + contentType + "\r\n";
    }
    if (contentLen) {
        resp += "Content-Length: " + std::to_string(contentLen) + "\r\n";
    }
    resp += "\r\n";
}

std::string urlDecode(std::string &SRC) {
    std::string ret;
    char ch;
    int i, ii;
    for (i = 0; i < SRC.length(); i++) {
        if (int(SRC[i]) == 37) {
            sscanf(SRC.substr(i + 1, 2).c_str(), "%x", &ii);
            ch = static_cast<char>(ii);
            ret += ch;
            i = i + 2;
        } else {
            ret += SRC[i];
        }
    }
    return (ret);
}

bool isSmthIndir(std::string &path) {
    try {
        for (const auto &entry : std::filesystem::directory_iterator(path))
            break;
    } catch (...) {
        return false;
    }
    for (const auto &entry : std::filesystem::directory_iterator(path)) {
//        std::cout << entry.path() << std::endl;
        return true;
    }
    return false;
}

response Server::getResp(std::string &req) {
    response resp;
    std::string path = getPath(req);
    if (path[path.size() - 1] == '/') {
        if (!isSmthIndir(path)) {
            resp.description = "404 Not Found\r\n";
            resp.len = 0;
            resp.file.clear();
            return resp;
        }
    }
    resp.file = getContent(path, resp.file);
    if (resp.file.empty()) {
        resp.description = "404 Not Found\r\n";
        resp.len = 0;
        resp.type = "";
    } else {
        resp.len = resp.file.size();
        resp.description = "200 OK\r\n";
    }

    resp.type = getType(path);
    if (resp.type.empty()) {
        resp.description = "403 Forbidden\r\n";
        resp.len = 0;
        resp.file.clear();
    }
    return resp;
}

std::string Server::getPath(std::string &req) {
    auto from = req.find(' ') + 1;
    std::string path = "../static";
    auto to = req.find(' ', from);
    if (to > req.find('?', from)) {
        to = req.find('?', from);
    }
    for (uint16_t i = from; i < to; i++) {
        path += req[i];
    }
    path = urlDecode(path);
    if (path[path.size() - 1] == '/') {
        std::vector<std::string> files;
        try {
            for (const auto &entry : std::filesystem::directory_iterator(path))
                break;
        } catch (...) {
            return path;
        }

        for (const auto &entry : std::filesystem::directory_iterator(path)) {
//            std::cout << entry.path() << std::endl;
            files.push_back(entry.path().string());
        }
        std::string first = files[0];
        for (const auto &path:files) {
            if (path[0] < first[0]) first = path;
            else if (path[0] > first[0]) continue;
            else {
                for (size_t i = 0; i < path.size(); ++i) {
                    if (i >= first.size()) break;
                    else if (path[i] > first[i])break;
                    else if (path[i] < first[i]) {
                        first = path;
                        break;
                    }
                }
            }
        }
        return first;
    }
    return path;
}

std::string Server::getType(std::string &path) {
    std::string type;
    for (int32_t i = path.size() - 1; i >= 0; i--) {
        if (path[i] == '.') {
            break;
        }
        type += path[i];
    }
    std::reverse(type.begin(), type.end());
    if (type == "txt")
        type = "text/txt";
    else if (type == "css")
        type = "text/css";
    else if (type == "html")
        type = "text/html";
    else if (type == "js")
        type = "application/javascript";
    else if (type == "jpg" || type == "jpeg")
        type = "image/jpeg";
    else if (type == "png")
        type = "image/png";
    else if (type == "gif")
        type = "image/gif";
    else if (type == "swf")
        type = "application/x-shockwave-flash";
    else
        type = "";
    return type;
}

std::string Server::getContent(std::string &path, std::string &read) {
    std::fstream file(path, std::ios::binary | std::ios::in);
    if (file.is_open()) {
        uint8_t a;
        do {
            a = file.get();
            if (file.eof())break;
            read.push_back(a);
        } while (true);
    }
    file.close();
    return read;
}

void Server::error(const char *msg) {
    perror(msg);
    exit(1);
}

//Content-Type for .js
//query string with get params