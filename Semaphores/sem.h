#ifndef _SEM_H_
#define _SEM_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <errno.h>


int getidsem(key_t key, int nsem);						/* uzyskaj identyfikator tablicy semaforow zawierajacej nsem semaforow */
void initialize(int idsem, int sem_num, int val);		/* zainicjuj semafor zadaną wartością val (SETVAL) */
void remove_shm(int idsem);									/* usun semafor */
void remove_sem(int idsem);
static void wywsem(int idsem, int sem_num, short op);	/* wywołanie semop */
void P(int idsem, int sem_num);							/* zajmij semafor */
void V(int idsem, int sem_num);							/* zwolnij semafor */
	


#endif
