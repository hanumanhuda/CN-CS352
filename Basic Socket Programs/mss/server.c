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

	ret=bind(sfd,(struct sockaddr*)&saddr,sizeof(saddr));	
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
		printf("started\n");
		int c;
		char type[10];
		recv(nsfd,type,strlen(type),0);
		c=fork();
		if(c==0)
		{
			dup2(nsfd,0);
			dup2(nsfd,1);
			close(sfd);
			if(strcmp(type,"1")==0)
			{
				execl("./s1","./s1",NULL);
			}
			else
			{
				execl("./s2","./s2",NULL);
			}
		}
		else
		{
			close(nsfd);
		}
	}

	return 0;
}
