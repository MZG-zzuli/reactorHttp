#pragma once
#include<stdbool.h>

enum FDEvent
{
    TimeOut = 0x01,
    ReadEvent = 0x02,
    WriteEvent = 0x04
};
typedef int(*handelFunc)(void* arg);
struct Channel{
    int fd;
    int events;
    handelFunc readCallBack;
    handelFunc writeCallBack;
    void* arg;
};
struct Channel* channelInit(int fd,int events,handelFunc readFun,handelFunc writeFun,void* arg);
void writeEventEnable(struct Channel* channel,bool flag);
bool isWriteEventEnable(struct Channel* channel);