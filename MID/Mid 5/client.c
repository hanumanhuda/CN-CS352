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

void echochat(int cfd,char *m)
{
	send(cfd,m,strlen(m)+1,0);
	while(1)
	{
		sleep(1);
		char *buf=(char*)malloc(BUF_SIZE);
		read(0,buf,BUF_SIZE);
		send(cfd,buf,strlen(buf),0);
		int ret=recv(cfd,buf,BUF_SIZE,0);
		if(ret==0)
			exit(1);
		if(strcmp(buf,"end")==0)
			break;
		printf("%s", buf);
	}
	/*int c=fork();
	if(c==0)
	{
		while(1)
		{
			sleep(1);
			char *buf=(char*)malloc(BUF_SIZE);
			read(0,buf,BUF_SIZE);
			send(cfd,buf,strlen(buf),0);
		}
	}
	else
	{
		while(1)
		{
			char *buf=(char*)malloc(BUF_SIZE);
			int ret=recv(cfd,buf,BUF_SIZE,0);
			if(ret==0)
				exit(1);
			if(strcmp(buf,"end")==0)
				break;
			printf("%s", buf);
			sleep(1);
		}
	}*/
}
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
	char *m=(char*)malloc(BUF_SIZE);
	ret=recv(cfd,m,BUF_SIZE,0);
	printf("%s\n",m );
	while(1)
	{
		char *buf;
		buf=(char*)malloc(BUF_SIZE);
		ret=recv(cfd,buf,BUF_SIZE,0);
		if(ret==0)
			break;

		if(strcmp(buf,"e")==0)
			echochat(cfd,m);
		printf("%s",buf);
		sleep(1);
	}
	return 0;
}