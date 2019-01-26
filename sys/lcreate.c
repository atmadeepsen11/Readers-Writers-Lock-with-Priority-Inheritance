#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>
#include <lock.h>

LOCAL int newlock();

SYSCALL lcreate(void)
{
        STATWORD ps;    
        int     lock;

        disable(ps);
        if ( (lock=newlock()) == SYSERR ) {
                restore(ps);
                return(SYSERR);
        }

        kprintf("lock is created %d\n ", lock );
	restore(ps);
        return(lock);
}

LOCAL int newlock()
{
        int     lock;
        int     i;
	kprintf("I am in lcreate \n");
        for (i=0 ; i<NLOCKS ; i++) {
                lock=nextlock--;
                if (nextlock < 0)
                        nextlock = NLOCKS-1;
                if (locktab[lock].lstate==LFREE || locktab[lock].lstate==DELETED) {
                        locktab[lock].lstate = LUSED;
                        return(lock);
                }
        }
        return(SYSERR);
}


