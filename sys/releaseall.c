#include <conf.h> 
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include <lock.h>

extern int perm;
extern unsigned long ctr1000;

int releaseall(int numlocks, int ldesc, ... ){
	int j=0;
	int args;
	for(j=0; j<numlocks; j++){
	args = *(&ldesc) + 1;	
//	STATWORD ps;
//	int retVal = OK;
//	int i, ldesc;
//	int lockid;
//	disable(ps);
//	for (i = 0; i < numlocks; i ++) {
//		ldesc = *(&args) + 1;
//		lockid = getlockid(ldesc);
//		if (!isbadlock(lockid) && locktab[lockid].lProc[currpid] == 1) { 
//			retVal = releaselock(currpid, lockid, 0);
//		} else {
//			retVal = SYSERR;
//		}
//	}
//	resched();
//	restore(ps);
//	return retVal;
// } 


// int releaselock(int pid, int ldesc, int toggle){ 

        STATWORD ps;
	struct lentry *lptr;
        struct pentry *pptr;
        int i=0;
        int nldesc;
        int flag=0;
        int return_val=0;
        int highest_prio=0;
        int index=0;
        disable(ps);
		
	pptr=&proctab[currpid];
        lptr=&locktab[ldesc];
	if(pptr->locktype[ldesc]<=0 || isbadlock(ldesc) || lptr->lstate==LFREE || lptr->lstate==DELETED)
           	return SYSERR;
//	kprintf("the type of lock and the type of process requesting lock %d, %s \n", pptr->locktype[ldesc], pptr->pname);
	if(lptr->lstate==LUSED){
//		 kprintf("the type of lock and the type of process releasing the lock %d, %s \n", pptr->locktype[ldesc], pptr->pname);
		if(pptr->locktype[ldesc]==ASSIGNED_R){
//		kprintf("hi i am here %s \n", pptr->pname);
//		 kprintf("the type of lock and the type of process requesting lock %d, %s \n", pptr->locktype[ldesc], pptr->pname);
			lptr->nreaders--;
//			kprintf("number of readers holding the lock the lock %d, \n", lptr->nreaders);
			index=q[lptr->lqtail].qprev;
//			 kprintf("who is waiting at the tail %s  %d, \n", proctab[index].pname, proctab[index].locktype[ldesc]);
			if(proctab[index].locktype[ldesc]==WAIT_W && lptr->nreaders==0){
//				kprintf("i as a writer will be put in %s , %d \n",  proctab[index].pname, proctab[index].locktype[ldesc] );
				proctab[index].locktype[ldesc]=ASSIGNED_W;
				lptr->nwriters++;
				locktab[ldesc].ltype=TAKEN_LOCK_W;
				flag=1;
//				kprintf("i as a writer will be given the lock %s , %d \n",  proctab[index].pname, proctab[index].locktype[ldesc] );
				dequeue(index);
		                ready(index, RESCHNO);
                		resched();
                		restore(ps);
                		return OK;
			}
		
		}
		else{
			index = q[lptr->lqtail].qprev;
			if(proctab[index].locktype[ldesc]==WAIT_W){
				proctab[index].locktype[ldesc]=ASSIGNED_W;
				locktab[ldesc].ltype=TAKEN_LOCK_W;
				flag=1;
			}else{
				index = q[lptr->lqtail].qprev;
				int temp=lptr->lqhead;
				while(index!=lptr->lqhead){
					if(proctab[index].locktype[ldesc]==WAIT_W){
						highest_prio = proctab[index].lprio;
						if(proctab[index].lprio==highest_prio && proctab[index].locktype[ldesc]==WAIT_R && (ctr1000-proctab[index].wait_time <50)){
							proctab[index].locktype[ldesc]=ASSIGNED_R;
                                                	locktab[ldesc].ltype=TAKEN_LOCK_R;
                                                	lptr->nreaders++;
                                                	dequeue(index);
                                                	ready(index, RESCHNO);
                                                	restore(ps);
                                                	return OK;
						}
					}
					else{
						if(temp!=index){	
							proctab[index].locktype[ldesc]=ASSIGNED_R;
							locktab[ldesc].ltype=TAKEN_LOCK_R;
							lptr->nreaders++;
							dequeue(index);
							ready(index, RESCHNO);
							restore(ps);
							return OK;
						}
					}
				}
			}
		}
		return_val=OK;
		return return_val;
	}
	else{
//		kprintf("hi i am here %s \n", pptr->pname);
		index=q[lptr->lqtail].qprev;
		if(proctab[index].locktype[ldesc]==WAIT_W && lptr->nreaders==0){
			proctab[index].locktype[ldesc]=ASSIGNED_W;
			locktab[ldesc].ltype=TAKEN_LOCK_W;
			flag=1;
		}else{
//			kprintf("hi i am here %s \n", pptr->pname);
			index = q[lptr->lqtail].qprev;
			int temp=lptr->lqhead;
			while(index!=lptr->lqhead){
				if(proctab[index].locktype[ldesc]==WAIT_W)
						temp = index;
				if(index!=temp){
					lptr->nreaders++;
					proctab[index].locktype[ldesc]=ASSIGNED_R;
					locktab[ldesc].ltype=TAKEN_LOCK_R;
					dequeue(index);
					ready(index, RESCHNO);
					restore(ps);
					return OK;						
				}
			}
		}
		return_val=OK;
	}
		
	if(flag==1){
//		kprintf("dequeue %s \n", pptr->pname);
		dequeue(index);
		ready(index, RESCHNO);
		resched();
		restore(ps);
		return return_val;
	}
		
	}	
}	
		






/*

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include <lock.h>

extern int perm;
extern unsigned long ctr1000;

void set_lstate(int ldesc){
        struct pentry *pptr;
        struct lentry *lptr;
        lptr=&locktab[ldesc];
        int tail=q[lptr->lqhead].qprev;
        int head=lptr->lqhead;
        int highest_prio=0;
        if(tail==head){
                lptr->lstate=LFREE;

        }else{	
		if(proctab[tail].locktype[ldesc]==WAIT_R){
                highest_prio=checkforprio(ldesc);
                	if(perm==1){
				struct pentry *proc=&proctab[highest_prio];
			}
                    	locktab[ldesc].ltype=TAKEN_LOCK_R;
                    	int index=q[lptr->lqtail].qprev;
//                    	while(proctab[index]!=proctab[highest_prio] && (index!=lptr->lqhead)){
                    	while(index!=highest_prio){
		                lptr->nreaders=lptr->nreaders+1;
				proctab[index].locktype[ldesc]=ASSIGNED_R;
				index=q[index].qprev;
  	                }
			index=q[highest_prio].qprev;
//			while(index!=lptr->lqhead){
			while(index!=lptr->lqhead){			
				if((proctab[index].locktype[ldesc]==WAIT_R) && (ctr1000-proctab[index].wait_time<500) && (proctab[index].lprio==proctab[highest_prio].lprio)){							lptr->nreaders=lptr->nreaders+1;
	  	      		proctab[index].locktype[ldesc]=ASSIGNED_R;
				}
				index=q[index].qprev;
			}
                }else if(proctab[tail].locktype[ldesc]==WAIT_W){
                        locktab[ldesc].ltype=TAKEN_LOCK_W;
			proctab[highest_prio].locktype[ldesc]=ASSIGNED_W;
                }
        }
}

int releaseall(int numlocks, int ldesc, ... ){
//	kprintf("I am in releaseall\n");
	STATWORD ps;
	struct lentry *lptr;
	struct pentry *pptr;
	int i=0;
	int nldesc;
	int flagforSignal;
	int return_val=0;
	int highest_prio;
	int index=0;
	disable(ps);
	
//	for(i=0; i<numlocks; i++){
//		int nldesc=*(&ldesc)+1;
		pptr=&proctab[currpid];
		lptr=&locktab[ldesc];
		kprintf("I am in releaseall\n");
		kprintf("which process type is holding my lock %s, %d\n", pptr->pname, pptr->locktype[ldesc]);
		if(pptr->locktype[ldesc]<=0)
			return_val=SYSERR;

		if(pptr->locktype[ldesc]==ASSIGNED_R){
			 kprintf("which process type is releasing the lock %s, %d\n", pptr->pname, pptr->locktype[ldesc]);
			lptr->nreaders-=1;
			if(lptr->nreaders!=0){
			kprintf("which process type is releasing the lock %s, %d\n", pptr->pname, pptr->locktype[ldesc]);
				flagforSignal=0;	
			}else
				set_lstate(ldesc);
		}else if(pptr->locktype[ldesc]==ASSIGNED_W)
			set_lstate(ldesc);
		pptr->wait_time=0;
		struct pentry *proc;
		if(lptr->lstate==LUSED){
			if(flagforSignal){
				highest_prio=checkforprio(ldesc);
				if(perm==0){
					proc=&proctab[highest_prio];
				}
				index=q[lptr->lqtail].qprev;
				if (&proctab[index] == highest_prio && highest_prio != &proctab[lptr->lqhead]){
					dequeue(index);
					ready(index, RESCHNO);
				}
				else{ 
					if (index != lptr->lqhead ){
						while ( index != highest_prio){
							dequeue(index);
							ready(index, RESCHNO);
							index =  q[index].qprev;
						}
					}
				}
			}
		}
//	}
	restore(ps);
	return return_val;
}
					
		
		
int checkforprio2(int ldesc) {
        struct lentry *lptr;
        struct pentry *pptr;
        lptr = &locktab[ldesc];
	int perm =0;
        int index = q[lptr->lqtail].qprev;
        while (index != lptr->lqhead) {
                pptr = &proctab[index];
                if (proctab[index].locktype[ldesc] == WAIT_W) {
                        perm=1;
                        return index;
                }
                index = q[index].qprev;
        }
        return -1;
}










int checkforprio(int ldesc) {
        struct lentry *lptr;
        struct pentry *pptr;
        lptr = &locktab[ldesc];
        int index = q[lptr->lqtail].qprev;
        while (index != index) {
                pptr = &proctab[index];
                if (proctab[index].locktype[ldesc] == WRITE) {
                        permission=1;
                        return index;
                }
                index = q[index].qprev;
        }
        return -1;
}


void set_lstate(int ldesc){
	struct pentry *pptr;
	struct lentry *lptr;
	lptr=&locktab[ldesc];
	int tail=q[locktab[lqtail]].qprev;
	int head=q[locktab[lqhead]];
	int highest_prio=0;
	if(tail==head){
		lptr->type=NOT_TAKEN_LOCK;
		
	else{
		if(proctab[currpid].locktype[ldesc]==TAKEN_LOCK_W)
			if(proctab[tail].locktype[ldesc]==WAIT_R){
				lptr->nreaders-=1;
				highest_prio=checkforprio(ldesc);
				if(permission==1 && lptr->nreaders==0){
					locktab[ldesc].ltype=TAKEN_LOCK_W;
					proctab[highest_prio].locktype[ldesc]=ASSIGNED_W;
				}
				else{
				
					locktab[ldesc].ltype=TAKEN_LOCK_R;
                        	        int index=q[lptr->lqtail].qprev;
					while(index!=lptr->lqhead){
						 proctab[highest_prio].locktype[ldesc]=ASSIGNED_R;
				}
				}else if(proctab[tail].locktype[ldesc]==WAIT_W){
					locktab[ldesc].ltype=TAKEN_LOCK_W;
                        		proctab[highest_prio].locktype[ldesc]=ASSIGNED_W;
				}		
			}
		}
	 
	
*/
