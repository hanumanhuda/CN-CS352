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
	if (argc!=2)
	{
		printf("Pass the main server Port\n");
		exit(1);
	}
	int ret,i;
	int sfd=socket(AF_INET,SOCK_STREAM,0);
	if (sfd==-1)
	{
		printf("Socket Error : %d\n", errno);
		exit(1);
	}

	struct sockaddr_in saddr;
	int port=atoi(argv[1]);
	memset(&saddr, 0, sizeof(struct sockaddr_in));
	saddr.sin_family=AF_INET;
	saddr.sin_port=htons(port);
	saddr.sin_addr.s_addr=INADDR_ANY;//inet_addr("127.0.0.1");
	int addr_len=sizeof(struct sockaddr_in);

	ret=bind(sfd,(struct sockaddr*)&saddr,sizeof(saddr));	
	if (ret==-1)
	{
		printf("Bind Error : %d\n",errno);
		exit(1);
	}

	ret=listen(sfd,5);
	if (ret==-1)
	{
		printf("Listen Error : %d\n", errno);
		exit(1);
	}
	int caddr_len=sizeof(struct sockaddr_in);
	i=0;
	while(1)
	{
			sleep(1);
			struct sockaddr_in caddr;
			memset(&caddr, 0, sizeof(struct sockaddr_in));
			int nsfd=accept(sfd,(struct sockaddr*)&caddr,&caddr_len);
			if(nsfd==-1)
			{
				printf("Accept Error : %d\n", errno);
				exit(1);
			}
			char msg[100];
			strcpy(msg,"Groups ");
			int j;
			for(j=1;j<=i;j++)
			{
				sprintf(msg,"%s%d\n",msg,j);
			}
			send(nsfd,msg,strlen(msg),0);
			char *type;
			type=(char*)malloc(10);
			recv(nsfd,type,10,0);
			j=atoi(type);
			if(j>i)
			{			
				int c;
				c=fork();
				int tport=port+j;
				int pp[2];
				ret=pipe(pp);
				if(ret==-1)
				{
					printf("Pipe Error %d\n",errno);
					exit(1);
				}
				if(c==0)
				{
					close(nsfd);
					sprintf(msg,"%d",tport);
					write(pp[1],msg,strlen(msg));
					close(pp[1]);
					dup2(pp[0],0);
					execl("./g","./g",NULL);
				}
				else
				{
					close(pp[0]);
					close(pp[1]);
				}
				i++;
			}
			sprintf(type,"%d",port+j);
			send(nsfd,type,strlen(type),0);
	}
	return 0;
}