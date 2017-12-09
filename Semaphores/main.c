#include "sem.h"
#include <time.h>
#define BUFFER_SIZE 10
#define K 6
#define M 10

typedef struct buffer_t
{
	int size;
	int* buf;
} buffer_t;

void producerTask();
void consumerTask();
int main(int argc, char *argv[]) {
	
	/*if (argc!=3)
	{
		printf("Wywolaj funkcje z 2 argumentami:\n   1) K - ilosc producentow,\n   2) M - ilosc buforow\n");
		return -1;
	}
	int K = atoi(argv[1]);
	int M = atoi(argv[2]);*/
	
	FILE *f = fopen("log.txt","w");
	/* Generate key for creating shared memory segment */
	key_t key = ftok(argv[0], 1);
	
	/* Create a shared memory segment */
	int idshmTable;
	//printf("sizeofbuffer_t: %d\n",sizeof(buffer_t));
	if ( (idshmTable = shmget(key,M*sizeof(buffer_t),0666 | IPC_CREAT) ) == -1)
		{printf("shmget failed, errno: %d\n", errno); exit(1);}
	
	enum {MUTEX, FULL, EMPTY};
	
	/* Create semaphore set for table of buffers */
	int idsemTable;
	key = ftok(argv[0], 2);
	idsemTable=getidsem(key,3);
	initialize(idsemTable,MUTEX,1);
	initialize(idsemTable,FULL,M);
	
	/* Create semaphore sets for every buffer */
	int idsemBuffer[M];
	for (int i=0; i<M; i++)
	{
		key = ftok(argv[0],3+i);
		idsemBuffer[i]=getidsem(key,3);
		initialize(idsemBuffer[i],MUTEX,1);
		initialize(idsemBuffer[i],EMPTY,BUFFER_SIZE);
		initialize(idsemBuffer[i],FULL,0);
	}
	//printf("idshmTable: %d\n",idshmTable);
	/* Create BufferTable */
	struct buffer_t * bufferTable;
	//printf("idshm: %d\n", idshmTable);
	if ((bufferTable = (void*)shmat(idshmTable,NULL,0)) == (void*) -1)
		{ printf("shmat failed, errno: %d\n",errno); exit(1); }
	
	int shmid_buf[M];
	for (int i=0; i<M; i++)
	{
		bufferTable[i].size=0;
		key=ftok(argv[0],3+M+i);
		if ((shmid_buf[i] = shmget(key,sizeof(int)*BUFFER_SIZE,0666 | IPC_CREAT)) == -1)
			{ printf("shmget failed here, errno: %d\n", errno); exit(1); }
	}
	if (shmdt(bufferTable) == -1)
		{ printf("shmdt failed\n"); exit(1); }
	
	/* Create shared mem for counter: it will help 
	 * to decide if the proc is the last one
	 * and can clean after everyone */
	int countershmid;
	key=ftok("sem.c",1);
	if ( (countershmid = shmget(key,sizeof(int),0666 | IPC_CREAT) ) == -1)
		{printf("shmget failed, errno: %d\n", errno); exit(1);}
	
	int* counter;
	if ((counter = (void*)shmat(countershmid,NULL,0)) == (void*) -1)
		{ printf("shmat failed, errno: %d\n",errno); exit(1); }
	
	*counter=0;
	
	if (shmdt(counter) == -1)
		{ printf("shmdt failed\n"); exit(1); }
		
		
	time_t t;
	/* Intializes random number generator to be used by consumer */
    srand((unsigned) time(&t));

    
	/* Fork 1 consumer process and K producer processes */
	pid_t childPID;
	pid_t consumerPID=0;
	pid_t parentPID=getpid();
		
	childPID = fork();
	if (childPID>=0) //fork succesfull
	{
		if (childPID==0) // child
		{
			consumerPID=getpid();
		}
		else //parent
		{
			for (int i = 0; i<K; i++)
			{
				childPID = fork();
				if (childPID<0)
				{
					printf("Fork failed!\n");
					return(1);
				}
				else
				{	if(childPID == 0)
					{
						break;
					}
				}
			}
		}
	}
	else
	{
		printf("Fork failed!\n");
		return(-1);
	}
	
	if(getpid()==parentPID)
		fprintf(f,"%d\n",M);
		
	for (int i=0; i<3; i++)
	{
		
		/* Consumer task */
		if (getpid()==consumerPID)
		{
			int nr = rand()%M;
			P(idsemTable,FULL);
			
			P(idsemBuffer[nr],FULL);
			P(idsemBuffer[nr],MUTEX);
			
			if ((bufferTable[nr].buf = shmat(shmid_buf[nr],NULL,0)) == (void*) -1)
				{ printf("shmat failed, errno: %d\n",errno); exit(1); }
			--(bufferTable[nr].buf);
			if (shmdt(++bufferTable[nr].buf) == -1)
				{ printf("shmdt failed, errno: %d\n", errno); exit(1); }
			V(idsemBuffer[nr],MUTEX);
			V(idsemBuffer[nr],EMPTY);
			V(idsemTable,FULL);
			
			P(idsemTable,MUTEX);	
			if ((bufferTable = shmat(idshmTable,NULL,0)) == (void*) -1)
				{ printf("shmat failed, errno: %d\n",errno); exit(1); }
			bufferTable[nr].size--;
			fprintf(f,"B%d C[%d]\n",nr,bufferTable[nr].size);
			fflush(f);
			if (shmdt(bufferTable) == -1)
				{ printf("shmdt failed\n"); exit(1); }
			V(idsemTable,MUTEX);
			
		}
		
		/* Producer task */
		if (consumerPID == 0 && getpid() != parentPID)
		{
			int nr=0;
			
			//P(idsemTable,FULL);
			P(idsemTable,MUTEX);
			
			if ((bufferTable = shmat(idshmTable,NULL,0)) == (void*) -1)
				{ printf("shmat failed\n"); exit(1); }
			for (int i=0; i<M; i++)
			{
				if (bufferTable[i].size<bufferTable[nr].size)
				{
					nr=i;
				}
				
			}
			bufferTable[nr].size++;
			if (shmdt(bufferTable) == -1)
				{ printf("shmat failed\n"); exit(1); }
			
			
			
			/*if ((bufferTable = shmat(idshmTable,NULL,0)) == (void*) -1)
				{ printf("shmat failed\n"); exit(1); }
			bufferTable[nr].size++;
			if (shmdt(bufferTable) == -1)
				{ printf("shmdt failed\n"); exit(1); }*/
			
			
			V(idsemTable,MUTEX);
			//V(idsemTable,FULL);	
				
			P(idsemBuffer[nr],EMPTY);
			P(idsemBuffer[nr],MUTEX);
			
			if ((bufferTable[nr].buf = shmat(shmid_buf[nr],NULL,0)) == (void*) -1)
				{ printf("shmat failed\n"); exit(1);}
			++(bufferTable[nr].buf);
			if (shmdt(--bufferTable[nr].buf) == -1)
				{ printf("shmdt failed\n, errno: %d",errno); exit(1);}
			V(idsemBuffer[nr],MUTEX);
			V(idsemBuffer[nr],FULL);
			
			P(idsemTable,MUTEX);			
			if ((bufferTable = shmat(idshmTable,NULL,0)) == (void*) -1)
				{ printf("shmat failed\n"); exit(1); }
			fprintf(f,"B%d P%d[%d]\n",nr,getpid()-parentPID,bufferTable[nr].size);
			fflush(f);
			if (shmdt(bufferTable) == -1)
				{ printf("shmdt failed\n"); exit(1); }	
			V(idsemTable,MUTEX);
			
		}
	}
	
	if ((counter = (void*)shmat(countershmid,NULL,0)) == (void*) -1)
		{ printf("shmat failed, errno: %d\n",errno); exit(1); }
	
	++(*counter);
	if((*counter)<K+2)
		return 0;
	
	if (shmdt(counter) == -1)
		{ printf("shmdt failed\n"); exit(1); }
		
	
	
	/* Cleaning! */
	remove_shm(idshmTable);
	remove_shm(countershmid);
	for (int i=0; i<M; i++)
		remove_shm(shmid_buf[i]);
	remove_sem(idsemTable);
	for (int i=0; i<M; i++)
		remove_sem(idsemBuffer[i]);
	
	fclose(f);
	return 0;
}


