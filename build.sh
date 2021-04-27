#!/bin/bash

g++ -Wall -g -c -std=c++20 ./main.cpp
g++ -Wall -g -c -std=c++20 ./server/Server.cpp

g++ main.o Server.o -lstdc++fs -pthread -std=c++20 -o main

rm *.o