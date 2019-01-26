#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>
#include <lock.h>

/*------------------------------------------------------------------------
 *  * ldelete  --  delete a lock by releasing its table entry
 *   *------------------------------------------------------------------------
 *    */

int ldelete( int ldesc ){
        STATWORD ps;
        int     pid;
        struct  lentry  *lptr;
        int i=0;
	lptr=&locktab[ldesc];
        disable(ps);
//	kprintf("im in delete\n");
	 if (isbadlock(ldesc) || locktab[ldesc].lstate==LFREE) {
                restore(ps);
                return(SYSERR);
         }
//	kprintf("im in delete\n");
	if(lptr->lstate==LUSED){
		lptr->lstate=DELETED;
//	kprintf("im in delete\n");
         while(i<NPROC){
                if(proctab[i].locktype[ldesc]){
                      proctab[i].locktype[ldesc]=-1;
		      kprintf("im in delete\n");
		}
	i++;
//		kprintf("im in delete %d\n", proctab[i].locktype[ldesc]);
        }
/*
*/
	if (nonempty(lptr->lqhead)) {
                while( (pid=getfirst(lptr->lqhead)) != EMPTY)
                  {
                    proctab[pid].pwaitret = DELETED;
                    ready(pid,RESCHNO);
                  }
                resched();
        }
        restore(ps);
        return(OK);

}
}


	
		

/*
SYSCALL ldelete(int ldesc)
{
        STATWORD ps;
        int     pid;
        struct  lentry  *lptr;
	int i=0;
        disable(ps);
        if (isbadlock(ldesc) || locktab[ldesc].lstate==LFREE) {
                restore(ps);
                return(SYSERR);
        }
        lptr = &locktab[ldesc];
        lptr->ltype = DELETED;
	while(i<NPROC){
		if(proctab[i].locktype[ldesc]==1)
			proctab[i].locktype[ldesc]=DELETED;
	}

        if (nonempty(lptr->lqhead)) {
                while( (pid=getfirst(lptr->lqhead)) != EMPTY)
                  {
                    proctab[pid].pwaitret = DELETED;
                    ready(pid,RESCHNO);
                  }
                resched();
        }
        restore(ps);
        return(OK);
}

*/
