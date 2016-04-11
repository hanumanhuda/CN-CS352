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

int nsfd[100];
int n=0;

void chat(void *arg)
{
	int i=0,j;
	while(n==0);
	while(1)
	{
		struct timeval tv;
	    tv.tv_sec = 1;
	    tv.tv_usec = 0;
	    fd_set tr;
	    FD_ZERO(&tr);
	    FD_SET(nsfd[i],&tr);
	    int res=select(nsfd[i]+1,&tr,NULL,NULL,&tv);
	    if(res>0&&FD_ISSET(nsfd[i],&tr))
	    {
	    	char *buf;
	    	buf=(char*)malloc(BUF_SIZE);
	    	recv(nsfd[i],buf,BUF_SIZE,0);
	 		char *msg=(char*)malloc(BUF_SIZE);
	 		sprintf(msg,"Sender %d : %s",i,buf);
	 		printf("%s\n", msg);   	
	    	for(j=0;j<n;j++)
	    		if(i!=j)
	    			send(nsfd[j],msg,strlen(msg),0);

	    }
	    sleep(1);
	    i=(i+1)%n;
	}
}
int main(int argc, char const *argv[])
{
	printf("Group started\n");
	char m[100];
	read(0,m,100);
	int port=atoi(m);
	printf("%d\n",port );
	int ret,i;
	int sfd=socket(AF_INET,SOCK_STREAM,0);
	if (sfd==-1)
	{
		printf("Socket Error : %d\n", errno);
		exit(1);
	}

	struct sockaddr_in saddr;
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
	pthread_t t;
	int nsfd[100];
	i=0;
	pthread_create(&t,NULL,&chat,NULL);
	while(1)
	{
		struct sockaddr_in caddr;
		memset(&caddr, 0, sizeof(struct sockaddr_in));
		nsfd[n]=accept(sfd,(struct sockaddr*)&caddr,&caddr_len);
		if(nsfd[n]==-1)
		{
			printf("Accept Error : %d\n", errno);
			exit(1);
		}
		n++;

		sleep(1);
	}
	return 0;
}