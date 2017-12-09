#include "sem.h"

int getidsem(key_t key, int nsem) 
{
	int idsem;
	if ((idsem = semget(key, nsem, 0666 | IPC_CREAT)) == -1)
		{printf("idsem failed\n"); exit(1);}
	return(idsem);
}

void initialize(int idsem, int sem_num, int val)
{
	if (semctl(idsem,sem_num,SETVAL,val) == -1)
		{printf("semctl failed\n"); exit(1);}
}

void remove_shm(int idshm)
{
	if (shmctl(idshm,IPC_RMID,NULL) == -1)
		{printf("shmctl failed, errno: %d\n", errno); exit(1);}
}

void remove_sem(int idsem)
{
	if (semctl(idsem,IPC_RMID,0) == -1)
		{printf("semctl failed\n"); exit(1);}
}

static void wywsem(int idsem, int sem_num, short op) 
{
	struct sembuf bs;
	
	bs.sem_num = sem_num;
	bs.sem_op = op;
	bs.sem_flg = 0;
	
	if (semop(idsem,&bs,1) == -1)
		{printf("semop failed\n"); exit(1);}
}

void P(int idsem, int sem_num)
{
	//printf("P: idsem %d sem_num %d PID %d\n",idsem,sem_num,getpid());
	wywsem(idsem,sem_num,-1);
}

void V(int idsem, int sem_num)
{
	//printf("V: idsem %d sem_num %d PID %d\n",idsem,sem_num,getpid());
	wywsem(idsem,sem_num,1);
}
