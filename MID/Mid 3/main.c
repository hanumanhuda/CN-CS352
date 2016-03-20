#include <stdio.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
int main(int argc, char const *argv[])
{
	/*/*FILE *f=popen("./p6","w");
	int fd1=fileno(f);
	int fd[2];
	pipe(fd);
	int c=fork();
	if(c==0)
	{
		close(fd[1]);
		dup2(fd[0],0);
		execl("p6","p6",NULL);
	}
	close(fd[0]);
	c=fork();
	if(c==0)
	{
		close(fd[0]);
		dup2(fd[1],1);
		execl("p2","p2",NULL);
	}
//	FILE *f1=popen("./p2","r");
//	int fd2=fileno(f1);

	int i=0;
	while(i<10)
	{
		sleep(1);
		i++;

	}*/
	int i,fd[4][2];
	char *prcs[]={"./p2","./p3","./p4","./p5"};
	struct pollfd pfd[4];
	for(i=0;i<4;i++)
	{
		pipe(fd[i]);
		int c=fork();
		if(c==0)
		{
			close(fd[i][0]);
			dup2(fd[i][1],1);
			execl(prcs[i],prcs[i],NULL);
		}
		close(fd[i][1]);
		pfd[i].fd=fd[i][0];
		pfd[i].events=POLLIN;
	}
	int npfd=4;
	int ret=poll(pfd,npfd,2);
	if(ret>0)
	{
		for(i=0;i<4;i++)
		if(pfd[i].revents==1)	
		{
			int c=fork();
			if(c==0)
			{
				dup2(pfd[i].fd,0);
				execl("p6","p6",NULL);
			}
			printf("%d\n", i);
			break;

		}
	}
	else
	{
		int tfd[2];
		pipe(tfd);
		int c=fork();
		if(c==0)
		{	
			close(tfd[1]);
			dup2(tfd[0],0);
			execl("p6","p6",NULL);
		}
		close(tfd[0]);
		dup2(tfd[1],1);
		printf("Hello\n");
	}

	return 0;
}
