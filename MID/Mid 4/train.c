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
	caddr.sin_addr.s_addr=INADDR_ANY;//inet_addr("127.0.0.1");
	int addr_len=sizeof(struct sockaddr_in);

	ret=connect(cfd,(struct sockaddr*)&caddr,addr_len);		
	if (ret==-1)
	{
		printf("Connect Error : %d\n", errno);
		exit(1);
	}
	char m[100];
	ret=recv(cfd,m,100,0);
	int port=atoi(m);
	int train_no=atoi(m+4);
	printf("%d\n",train_no );
	close(cfd);

	sleep(1);

	cfd=socket(AF_INET,SOCK_STREAM,0);
	if (cfd==-1)
	{
		printf("Socket Error : %d\n", errno);
		exit(1);
	}

	memset(&caddr, 0, sizeof(struct sockaddr_in));
	caddr.sin_family=AF_INET;
	caddr.sin_port=htons(port);
	caddr.sin_addr.s_addr=INADDR_ANY;//inet_addr("127.0.0.1");
	addr_len=sizeof(struct sockaddr_in);
	printf("%d\n",port );
	ret=connect(cfd,(struct sockaddr*)&caddr,addr_len);
	if (ret==-1)
	{
		printf("Connect Error : %d\n", errno);
		exit(1);
	}
		
	char *buf=(char*)malloc(255);
	sprintf(buf,"Train No. : %d and  E S1 S2 S3 S4 S5 S6 S7 S9 S10 A1 A2 A3 B1 B2\n",train_no);
	send(cfd,buf,strlen(buf),0);
	recv(cfd,buf,BUF_SIZE,0);
	printf("Train %d the platform \n", train_no);
	return 0;
}