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

#define BUF_SIZE 255

int main(int argc, char  *argv[])
{
	char buf[200];
	printf("s3 is started\n");
	read(STDIN_FILENO,buf,200);
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
			int input=atoi(in);
            input=input*input*input*input;
            //sleep(50);
            char *out;
            out=(char*)malloc(BUF_SIZE);
            sprintf(out,"%d\n",input);
            out[strlen(out)]='\0';
            write(fd2,out,strlen(out)+1);
            printf("s3 : finished\n");
            break;
		}
	}
}