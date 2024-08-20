//
// Created by admin on 24-8-9.
//

#include "server.h"
#include<string.h>
#include<sys/stat.h>
#include<sys/sendfile.h>
#include<signal.h>
#include<dirent.h>
#include<sys/types.h>
int initListenFd(int port) {
    int lfd=socket(AF_INET,SOCK_STREAM,0);
    if(lfd==-1) {
        perror("socket init error\n");
        return -1;
    }
    int opt=1;
    int ret=setsockopt(lfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof opt);
    if(ret==-1) {
        perror("setsockopt error\n");
        return -1;
    }
    struct sockaddr_in addr;
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=INADDR_ANY;
    addr.sin_port=htons(port);
    ret=bind(lfd,(struct sockaddr*)&addr,sizeof addr);
    if(ret==-1) {
        perror("bind error\n");
        return -1;
    }
    ret=listen(lfd,128);
    if(ret==-1) {
        perror("listen error\n");
        return -1;
    }

    return lfd;
}
int epollRun(int lfd) {
    int epfd=epoll_create(1);
    if(epfd==-1) {
        perror("create epoll error\n");
        return -1;
    }
    struct epoll_event ev;
    ev.events=EPOLLIN;
    ev.data.fd=lfd;
    int ret=epoll_ctl(epfd,EPOLL_CTL_ADD,lfd,&ev);
    if(ret==-1) {
        perror("epoll ctl add error\n");
        return -1;
    }
    struct epoll_event evs[1024];
    while(1) {
        int nums=epoll_wait(epfd,evs,sizeof(evs)/sizeof(struct epoll_event),-1);
        for(int i=0;i<nums;i++) {
            if(evs[i].data.fd==lfd) {
                acceptClient(lfd,epfd);
            }else {
                recvHttpRequest(evs[i].data.fd,epfd);
            }
        }
    }
    return 0;
}
int acceptClient(int lfd,int epfd) {

    int cfd=accept(lfd,NULL,NULL);
    if(cfd==-1) {
        perror("accept error\n");
        return -1;
    }
    int flag=fcntl(cfd,F_GETFL);
    flag=flag|O_NONBLOCK;
    fcntl(cfd,F_SETFL,flag);
    struct epoll_event ev;
    ev.data.fd=cfd;
    ev.events=EPOLLIN|EPOLLET;
    int ret=epoll_ctl(epfd,EPOLL_CTL_ADD,cfd,&ev);
    if(ret==-1) {
        perror("epoll_ctl error\n");
        return -1;
    }
    return 0;
}

int recvHttpRequest(int cfd,int epfd) {
    int len=0,total=0;
    char tmp[1024]={0};
    char buff[4096*3]={0};
    while((len=recv(cfd,tmp,sizeof tmp,0))>0) {
        if(len+total<sizeof buff) {
            memcpy(buff,tmp,len);

        }
        total+=len;
    }
    if(len==-1&&errno==EAGAIN) {
        //解析请求行
        parseRequestLine(buff,cfd);

    }else if(len==0) {      //客户端关闭
        //epoll_ctl(epfd,EPOLL_CTL_DEL,cfd,NULL);
        //close(cfd);
    }else {
        perror("recv error\n");
    }
    epoll_ctl(epfd,EPOLL_CTL_DEL,cfd,NULL);
    close(cfd);
    return 0;
}

int parseRequestLine(const char* line,int cfd) {
    char path[1024]={0};
    char method[12]={0};
    sscanf(line,"%[^ ] %[^ ]",method,path);
    printf("path:%s\n",path);
    if(strcasecmp(method,"get")!=0) {
        return -1;
    }
    char* file=NULL;
    if(strcmp(path,"/")==0) {
        file="./";
    }
    else {
        file=path+1;
    }
    struct stat st;
    int ret=stat(file,&st);
    //no exit
    if(ret==-1) {
        sendHeadMsg(cfd,404,"Not Found",getFileType(".html"),-1);
        sendFile("404.html",cfd);
    }
    //is directory
    if(S_ISDIR(st.st_mode)) {
        if(file[strlen(file)-1]!='/')file[strlen(file)]='/';
        sendHeadMsg(cfd,200,"OK",getFileType(".html"),-1);
        sendDir(file,cfd);

    }else {     //is file,send file to cfd
        sendHeadMsg(cfd,200,"OK",getFileType(file),st.st_size);
        sendFile(file,cfd);
        //close(cfd);
    }
    return 0;
}

const char* getFileType(const char* name)
{
    // a.jpg a.mp4 a.html
    // 自右向左查找‘.’字符, 如不存在返回NULL
    const char* dot = strrchr(name, '.');
    if (dot == NULL)
        return "text/plain; charset=utf-8";	// 纯文本
    if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0)
        return "text/html; charset=utf-8";
    if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0)
        return "image/jpeg";
    if (strcmp(dot, ".gif") == 0)
        return "image/gif";
    if (strcmp(dot, ".png") == 0)
        return "image/png";
    if (strcmp(dot, ".css") == 0)
        return "text/css";
    if (strcmp(dot, ".au") == 0)
        return "audio/basic";
    if (strcmp(dot, ".wav") == 0)
        return "audio/wav";
    if (strcmp(dot, ".avi") == 0)
        return "video/x-msvideo";
    if (strcmp(dot, ".mov") == 0 || strcmp(dot, ".qt") == 0)
        return "video/quicktime";
    if (strcmp(dot, ".mpeg") == 0 || strcmp(dot, ".mpe") == 0)
        return "video/mpeg";
    if (strcmp(dot, ".vrml") == 0 || strcmp(dot, ".wrl") == 0)
        return "model/vrml";
    if (strcmp(dot, ".midi") == 0 || strcmp(dot, ".mid") == 0)
        return "audio/midi";
    if (strcmp(dot, ".mp3") == 0)
        return "audio/mpeg";
    if (strcmp(dot, ".ogg") == 0)
        return "application/ogg";
    if (strcmp(dot, ".pac") == 0)
        return "application/x-ns-proxy-autoconfig";

    return "text/plain; charset=utf-8";
}

int sendFile(const char* fileName,int cfd) {
    int f_fd=open(fileName,O_RDONLY);
    char buffer[4096]={0};
    if(f_fd<=0) {
        perror("file open error\n");
        return -1;
    }
    int size=lseek(f_fd,0,SEEK_END);
    lseek(f_fd,0,SEEK_SET);
    off_t offset=0;
    while(offset<size) {
        int ret=sendfile(cfd,f_fd,&offset,size-offset);
        // if(ret==-1&& errno == EAGAIN) {
        //     perror("senfile error\n");
        // }
        if(errno==EPIPE)
        {
            break;
        }
         printf("ret:%d\nerrno:%d\n",ret,errno);
        // printf("sendfile:%d,offset=%ld,size=%d\n",ret,offset,size);
    }
    close(f_fd);
    return 0;
}
int sendHeadMsg(int cfd,int status,const char* descr,const char* type,int length) {
    char buf[4096]={0};

    sprintf(buf,"http/1.1 %d %s\r\n",status,descr);
    sprintf(buf+strlen(buf),"content-type: %s\r\n",type);
    sprintf(buf+strlen(buf),"content-length: %d\r\n\r\n",length);
    send(cfd,buf,strlen(buf),0);
    perror("headMsg:");

    return 0;

}
int sendDir(const char* dirName,int cfd)
{
// int scandir(const char *dirp, struct dirent ***namelist,
//               int (*filter)(const struct dirent *),
//               int (*compar)(const struct dirent **, const struct dirent **));
    char buf[4096]={0};
    sprintf(buf,"<html><head><title>%s</title></head><body><table>",dirName);
    send(cfd,buf,strlen(buf),0);
    struct dirent** namelist;
    int num=scandir(dirName,&namelist,NULL,alphasort);
    memset(buf,0,sizeof buf);
    for(int i=0;i<num;i++)
    {
        char fileName[1024]={0};
        sprintf(fileName,"%s/%s",dirName,namelist[i]->d_name);
        struct stat st;
        stat(fileName,&st);
        if(S_ISDIR(st.st_mode))
        {
            sprintf(buf,
            "<tr><td><a href=\"/%s%s\">%s</a></td><td>%ld</td></tr>",
            dirName,namelist[i]->d_name,namelist[i]->d_name,st.st_size);
        }
        else
        {
            sprintf(buf,
            "<tr><td><a href=\"/%s%s\">%s</a></td><td>%ld</td></tr>",
            dirName,namelist[i]->d_name,namelist[i]->d_name,st.st_size);
        }
        int ret=send(cfd,buf,strlen(buf),0);
        printf("ret=%d,%s\n",ret,buf);
        memset(buf,0,sizeof buf);
    }
    sprintf(buf,"</table></body></html>");
    send(cfd,buf,strlen(buf),0);
}
