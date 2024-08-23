#include <stdio.h>
#include <unistd.h>
#include"server.h"
#include<signal.h>
#define port 80
#define path "/"
int main() {
    signal(SIGPIPE, SIG_IGN);
    int lfd=initListenFd(port);
    chdir(path);
    printf(" initsocket listen%d\n",lfd);

    epollRun(lfd);
    return 0;
}
