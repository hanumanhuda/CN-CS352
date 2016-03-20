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
 int main()
 {
 	int i=100;
 	printf("started 4\n");
 	while(i>0)
 	{
 		write(1,"This is process 4\n",strlen("This is process 4\n"));
 		i--;
 			sleep(6);
 	}
 }