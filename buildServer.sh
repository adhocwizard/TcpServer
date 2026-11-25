#!/bin/sh

g++ -O0 -g -o tcpServer -I include src/tcpServer.cc src/TcpServer.cc -lpthread

