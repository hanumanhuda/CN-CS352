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
#define BUF_SIZE 255

int main(int argc, char const *argv[])
{
	int ret;
	if (argc!=2)
	{
		printf("Pass Port Number\n");
		exit(1);
	}
	int cfd=socket(AF_INET,SOCK_STREAM,0);
	if (cfd==-1)
	{
		printf("Socket Error : %d\n", errno);
		exit(1);
	}

	struct sockaddr_in caddr;
	memset(&caddr, 0, sizeof(struct sockaddr_in));
	caddr.sin_family=AF_INET;
	caddr.sin_port=htons(atoi(argv[1]));
	caddr.sin_addr.s_addr=inet_addr("127.0.0.1");
	int addr_len=sizeof(struct sockaddr_in);

	ret=connect(cfd,(struct sockaddr*)&caddr,addr_len);		
	if (ret==-1)
	{
		printf("Connect Error : %d\n", errno);
		exit(1);
	}
	char m[10];
	ret=recv(cfd,m,10,0);
	if(strcmp(m,"-1")==0)
	{
		printf("Service is not available\n");
		exit(1);
	}
	while(1)
	{
		printf("Enter : \n");
		char *buf;
		buf=(char*)malloc(BUF_SIZE);
		read(0,buf,BUF_SIZE);
		ret=send(cfd,buf,strlen(buf),0);
		if(ret==-1)
		{
			printf("Send Error\n");
			break;
		}
		ret=recv(cfd,buf,BUF_SIZE,0);
		if(ret==-1)
		{
			printf("Recv Error\n");
			break;
		}
		printf("Result : %s",buf );
		sleep(1);
	}
	return 0;
}