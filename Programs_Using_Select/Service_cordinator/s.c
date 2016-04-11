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
struct sembuf sign;
int main(int argc, char const *argv[])
{
    if(argc!=2)
    {
        printf("Pass The server Number\n");
        exit(1);
    }
    int num=atoi(argv[1]);
    //semaphore variable
    int k4=ftok(".",12);
    sem=semget(k4,1,0777|IPC_CREAT);
    if(sem==-1)
    {
        printf("semget error\n");;
        exit(1);
    }
    semctl(sem,0,SETVAL,No_of_Servers);
    sign.sem_num=0;
    sign.sem_flg=0;
    sign.sem_op=1;

    //reading Semaphore array
    int pid;
    int k3=ftok(".",4);
    pid=shmget(k3,2*sizeof(int),0777|IPC_CREAT);
    if(pid==-1)
    {
        printf("shmget S error\n");
        exit(1);
    }
    s=shmat(pid,NULL,0);
   
    if(s==(int*)-1)
    {
        printf("shmat S error%d\n",*s);
        exit(1);
    }

    //Accessing fifonames
    int shmid;
    k3=ftok(".",3);
    shmid=shmget(k3,sizeof(struct Data*),0777|IPC_CREAT);
    if(shmid==-1)
    {
        printf("shmget Erro\n");
        exit(1);
    }
    struct Data *d;
    d=(struct Data*)shmat(shmid,NULL,0);
   
    if(d==(struct Data*)-1)
    {
        printf("shmat D error\n");
        exit(1);
    }
    printf("Started %d\n",num);
    while(1)
    {   
        sleep(2);
        if(s[num]==1)
        {   
            //printf("data: %d %d\n",d->in,d->out);
            char fifoname[30];
            strcpy(fifoname,d->name[d->in]);
            //printf("%s\n",fifoname );
            d->in++;
            int fd1,fd2;
            fd1=open(fifoname,O_RDONLY);
            sprintf(fifoname,"%s%d",fifoname,1);
            fd2=open(fifoname,O_WRONLY);
            char *buf=(char*)malloc(BUF_SIZE);
            read(fd1,buf,BUF_SIZE);
            int input=atoi(buf);
            input=input*input;
            //sleep(50);
            buf=(char*)malloc(BUF_SIZE);
            sprintf(buf,"%d",input);
            buf[strlen(buf)]='\0';
            write(fd2,buf,strlen(buf)+1);
            s[num]=0;
            semop(sem,&sign,1);

        }
    }
	return 0;
}