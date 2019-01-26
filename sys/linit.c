#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include <lock.h>

struct lentry locktab[NLOCKS];
int nextlock;
int prochold[NPROC];

void linit()
{
	struct lentry *lptr;
	int i, j;
	nextlock = NLOCKS-1;

	for(i = 0; i < NLOCKS; i++)
	{
		lptr = &locktab[i];
		lptr ->lstate = LFREE;
		lptr -> locknum = i;
		lptr -> lqtail = 1 + (lptr -> lqhead = newqueue());
		lptr -> nreaders = 0;
		lptr -> nwriters = 0;
		lptr -> maxlprio = 0;
		lptr -> ltype = NOT_TAKEN_LOCK ;		 
	}

	for(j = 0; j < NPROC; j++){
		prochold[j] = 0;
	}	
}
