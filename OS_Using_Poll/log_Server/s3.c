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
#include <unistd.h>
#include <errno.h>
#define BUF_SIZE 255

int main(int argc, char  *argv[])
{
	char txt[BUF_SIZE],buf[BUF_SIZE],fifoname[BUF_SIZE];
	printf("s3 is started\n");
	read(STDIN_FILENO,txt,BUF_SIZE);
	int i;
	for(i=0;i<strlen(txt);i++)
	{
		if(txt[i]==':')
		{
			buf[i]='\0';
			i++;
			break;
		}
		buf[i]=txt[i];
	}
	int j=0;
	while(i<strlen(txt))
		fifoname[j++]=txt[i++];
	fifoname[j]='\0';
	//printf("%s\n",fifoname );
	int fd=open(fifoname,O_WRONLY);
	if(fd<0)
	{
		printf("Open Error %d\n",errno);
		exit(1);
	}
	char fifo[BUF_SIZE];
	sprintf(fifo,"%s%d",buf,1);
	int fd1,fd2;
	fd1=open(buf,O_RDONLY);
	fd2=open(fifo,O_WRONLY);
	while(1)
	{
		sleep(2);
		char in[BUF_SIZE];
		read(fd1,in,BUF_SIZE);
		if(strlen(in)>0)
		{
			char t[BUF_SIZE];
			strcpy(t,in);
			strcat(t,"\n\0");
			write(fd,in,strlen(in)+1);
			int input=atoi(in);
            input=input*input*input*input;
            //sleep(50);
            char *out;
            out=(char*)malloc(BUF_SIZE);
            sprintf(out,"%d\n",input);
            sleep(2);
            out[strlen(out)]='\0';
            write(fd2,out,strlen(out)+1);
            write(fd,out,strlen(out)+1);
            printf("s3 : finished\n");
            break;
		}
	}
}