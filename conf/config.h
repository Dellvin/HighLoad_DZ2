//
// Created by dellv on 07.03.2021.
//

#ifndef DELLVINX_CONFIG_H
#define DELLVINX_CONFIG_H

#include <cstdint>

#include <cstdint>
#include <functional>
#include <thread>
#include <list>

#ifdef _WIN32 // Windows NT

#include <WinSock2.h>

#else // *nix

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#endif

#define DEFAULT_PORT 8000

#define BUFF_SIZE 2048

#define STATIC "../static"

#endif //DELLVINX_CONFIG_H
