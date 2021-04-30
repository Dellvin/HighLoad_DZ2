#!/bin/bash

g++ -Wall -g -c -std=c++17 ./main.cpp
g++ -Wall -g -c -std=c++17 ./server/Server.cpp

g++ main.o Server.o -pthread -o main

rm *.o