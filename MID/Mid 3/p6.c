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
 	printf("started\n");
 	while(i>0)
 	{
 		sleep(1);
 		char *buf;
 		buf=(char*)malloc(255);

 		read(0,buf,255);
 		printf("Recived By process 6 : %s\n", buf);
 		i--;
 	}
 }