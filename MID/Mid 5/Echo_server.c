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
#define No_of_Ports 4

int *flag;
void shm_init()
{
	int k=ftok(".",1);
	int shmid=shmget(k,sizeof(int)*100,0777|IPC_CREAT);
	if (shmid==-1)
	{
		printf("Shmid Error : %d\n",errno);
		exit(1);
	}
	flag=(int*)shmat(shmid,NULL,0);
	if(flag==(int*)-1)
	{
		printf("Shmat Error : %d\n",errno);
		exit(1);	
	}
}


int main(int argc, char const *argv[])
{
	int j=0;
	shm_init();

	char *m=(char*)malloc(BUF_SIZE);
	int client_no;
	write(1,"e",strlen("e")+1);
	read(0,m,BUF_SIZE);
	client_no=atoi(m);
	while(j<10)
	{
		char *buf=(char*)malloc(BUF_SIZE);
		read(0,buf,BUF_SIZE);
		char *msg=(char*)malloc(BUF_SIZE);
		sprintf(msg,"Echo back : %s",buf);
		if(j==10)
			break;
		write(1,msg,strlen(msg));
		/*struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 0;
	    fd_set tr;
	    FD_ZERO(&tr);
	    FD_SET(0,&tr);
	    int res=select(1,&tr,NULL,NULL,&tv);
	    if(res>0&&FD_ISSET(0,&tr))
	   	{
	   		
	   		int i;
			/*for(i=0;i<strlen(buf);i++)
			if(buf[i]>=97&&buf[i]<=122)
				buf[i]=buf[i]-32;
			write(1,buf,strlen(buf));
		}
		*/
		sleep(1);

		
		j++;
	}
	sleep(3);
	write(1,"end",4);
	printf("client_no : %d\n",client_no );
	flag[client_no]=0;
	return 0;
}