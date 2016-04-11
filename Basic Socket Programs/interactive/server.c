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
	int ret;
	if (argc!=2)
	{
		printf("Pass Port Number\n");
		exit(1);
	}
	int sfd=socket(AF_INET,SOCK_STREAM,0);
	if (sfd==-1)
	{
		printf("Socket Error : %d\n", errno);
		exit(1);
	}

	struct sockaddr_in saddr,caddr;
	memset(&saddr, 0, sizeof(struct sockaddr_in));
	memset(&caddr, 0, sizeof(struct sockaddr_in));
	saddr.sin_family=AF_INET;
	saddr.sin_port=htons(atoi(argv[1]));
	saddr.sin_addr.s_addr=inet_addr("127.0.0.1");
	int addr_len=sizeof(struct sockaddr_in);

	ret=bind(sfd,(struct sockaddr*)&saddr,addr_len);	
	if (ret==-1)
	{
		printf("Bind Error : %d\n", errno);
		exit(1);
	}

	ret=listen(sfd,3);
	if (ret==-1)
	{
		printf("Listen Error : %d\n", errno);
		exit(1);
	}
	int caddr_len;
	while(1)
	{
		sleep(1);
		int nsfd=accept(sfd,(struct sockaddr*)&caddr,&caddr_len);
		if(nsfd==-1)
		{
			printf("Accept Error : %d\n", errno);
			exit(1);
		}	
		printf("Started\n");
		char *buf;
		while(1)
		{
			buf=(char*)malloc(BUF_SIZE);
			ret=recv(nsfd,buf,BUF_SIZE,0);
			if(ret==0)
				break;
			int i;
			for(i=0;i<strlen(buf);i++)
			if(buf[i]>=97&&buf[i]<=122)
				buf[i]=buf[i]-32;

			send(nsfd,buf,strlen(buf),0);
			sleep(1);
		}
		printf("Finished\n");
	}

	return 0;
}