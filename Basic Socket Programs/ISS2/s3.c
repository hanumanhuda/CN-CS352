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

int *cnt;
void get_shm()
{
	int k=ftok(".",1);
	int shmid=shmget(k,sizeof(int)*No_of_Ports,0777|IPC_CREAT);
	if (shmid==-1)
	{
		printf("Shmid Error : %d\n",errno);
		exit(1);
	}
	cnt=(int*)shmat(shmid,NULL,0);
	if(cnt==(int*)-1)
	{
		printf("Shmat Error : %d\n",errno);
		exit(1);	
	}
}
int main(int argc, char const *argv[])
{
	printf("S3 started\n");
	get_shm();
	while(1)
	{
		char *buf=(char*)malloc(BUF_SIZE);
		int ret=read(0,buf,BUF_SIZE);
		if(ret==0)
		{
			cnt[2]++;
			break;
		}
		int i=0,j=strlen(buf)-2;
		while(j>i)
		{
			char ch=buf[i];
			buf[i]=buf[j];
			buf[j]=ch;
			i++;
			j--;
		}
		buf[strlen(buf)]='\0';
		write(1,buf,strlen(buf));
		sleep(1);
	}
	return 0;
}