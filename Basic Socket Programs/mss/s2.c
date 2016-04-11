#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <poll.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define BUF_SIZE 255
int main(int argc, char const *argv[])
{

	while(1)
	{
		char *buf=(char*)malloc(BUF_SIZE);
		int ret=read(0,buf,BUF_SIZE);
		if(ret==0)
			break;
		int i;
		for(i=0;i<strlen(buf);i++)
		if(buf[i]>=65&&buf[i]<=90)
			buf[i]=buf[i]+32;
		write(1,buf,strlen(buf));
		sleep(1);
	}
	return 0;
}