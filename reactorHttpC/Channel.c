#include"Channel.h"

struct Channel *channelInit(int fd, int events, handelFunc readFun, handelFunc writeFun, void *arg)
{
    struct Channel* channel=(struct Channel*)malloc(sizeof(struct Channel));
    channel->fd=fd;
    channel->events=events;
    channel->readCallBack=readFun;
    channel->writeCallBack=writeFun;
    channel->arg=arg;
    return channel;
}

void writeEventEnable(struct Channel *channel, bool flag)
{
    if(flag)
    {
        channel->events=channel->events|WriteEvent;
    }else
    {
        channel->events=channel->events& ~WriteEvent;
    }
}

bool isWriteEventEnable(struct Channel *channel)
{
    return channel->events&WriteEvent;
}
