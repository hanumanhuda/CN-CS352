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
#include <unistd.h>
#include <sys/un.h>
#include <fcntl.h>
#include <assert.h>
#define BUF_SIZE 255
int main(int argc, char const *argv[])
{
	int rsfd;
	rsfd=socket(AF_INET,SOCK_RAW,146);
	if (rsfd==-1)
	{
		printf("socket Error %d\n",errno );
		exit(1);
	}
	int addr_len=sizeof(struct sockaddr_in);
	struct sockaddr_in addr,s_addr,d_addr;
	addr.sin_port=0;//htons(atoi(argv[1]));
	addr.sin_family=AF_INET;
	addr.sin_addr.s_addr=inet_addr("127.0.0.1");
	int ret;
	struct iphdr *iph;
	unsigned int iphlen;
	while(1)
	{
		char *buf=(char*)malloc(BUF_SIZE);
		read(0,buf,BUF_SIZE);
		ret=sendto(rsfd,buf,BUF_SIZE,0,(struct sockaddr*)&addr,addr_len);
		if(ret<0)
		{
			printf("Send Error %d\n",errno );
			exit(1);
		}
		
	}
	return 0;
}