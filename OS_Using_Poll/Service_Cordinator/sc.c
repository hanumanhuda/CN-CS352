#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <poll.h>
#include <fcntl.h>
#include <string.h>
#define BUF_SIZE 255
#define No_of_Servers 2
char *name[100];
int cnt=0;
struct sembuf wait;
int sem;
int *s;
struct Data
{
    char name[30][30];
    int in;
    int out;
};
void handler()
{
    semop(sem,&wait,1);
    int i;
    int pid;
    int k3=ftok(".",4);
    pid=shmget(k3,sizeof(int)*No_of_Servers,0777|IPC_CREAT);
    s=(int*)shmat(pid,NULL,0);
   
    if(s==(int*)-1)
    {
        printf("semat error\n");
    }
    for(i=0;i<No_of_Servers;i++)
    {
        printf("%d\n",s[i] );
        if(s[i]==0)
        {printf("inside %d\n",i);
            s[i]=1;
            break;
        }
    }

}
int main(int argc, char const *argv[])
{
    //shared memory to write Process ID
	int pid;
    int k3=ftok(".",6);
    pid=shmget(k3,sizeof(int),0777|IPC_CREAT);
    int *p;
    p=shmat(pid,NULL,0);
   
    if(p==(int*)-1)
    {
    	printf("semat error\n");
    }
    (*p)=getpid();

    //attaching handler to SIGUSR1
    signal(SIGUSR1,handler);
    int k4=ftok(".",12);
    sem=semget(k4,1,0777|IPC_CREAT);
    if(sem==-1)
    {
        printf("semget error\n");;
        exit(1);
    }
    semctl(sem,0,SETVAL,No_of_Servers);
    wait.sem_num=0;
    wait.sem_flg=0;
    wait.sem_op=-1;


    int i;
    k3=ftok(".",4);
    pid=shmget(k3,sizeof(int)*No_of_Servers,0777|IPC_CREAT);
    s=(int*)shmat(pid,NULL,0);
   
    if(s==(int*)-1)
    {
        printf("semat error\n");
    }
    for(i=0;i<No_of_Servers;i++)
        s[i]=0;


    int shmid;
    k3=ftok(".",3);
    shmid=shmget(k3,sizeof(struct Data*),0777|IPC_CREAT);
    struct Data *d;
    d=(struct Data*)shmat(shmid,NULL,0);
   
    if(d==(struct Data*)-1)
    {
        printf("semat error\n");
    }
    d->in=0;
    d->out=0;
    printf("Started Main Server %d %d\n",d->in,d->out );
    while(1)
    {
        sleep(5);
    }
	return 0;
}