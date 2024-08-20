//
// Created by admin on 24-8-9.
//
#ifndef SERVER_H
#define SERVER_H




#include <stdio.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/epoll.h>
#include<fcntl.h>
#include<unistd.h>
#include<errno.h>


int initListenFd(int port);
int epollRun(int lfd);
int acceptClient(int lfd,int epfd);
int recvHttpRequest(int cfd,int epfd);
int parseRequestLine(const char* line,int cfd);
const char* getFileType(const char* name);
int sendFile(const char* fileName,int cfd);
int sendHeadMsg(int cfd,int status,const char* descr,const char* type,int length);
int sendDir(const char* dirName,int cfd);
int hexToDec(char c);
void decodeMsg(char* to, char* from);
#endif //SERVER_H
