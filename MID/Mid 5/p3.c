#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#define BUF_SIZE 255
int main(int argc, char const *argv[])
{
	int i=0;
	while(1)
	{
		sleep(5);
		write(1,"This is by P3\n",strlen("This is by P3\n"));

		i++;
		
	}
	return 0;
}