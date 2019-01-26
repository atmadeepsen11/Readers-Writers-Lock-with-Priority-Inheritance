#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include <lock.h>


extern long ctr1000;
extern int permission;


int lock (int ldesc, int type, int priority) {

	STATWORD ps;
	struct lentry *lptr;
    	struct pentry *pptr;
    	pptr = &proctab[currpid];
    	lptr = &locktab[ldesc];
    	int lid=getlid(ldesc);
	int highest_prio=0;
	int index=0;
	disable(ps);
		
//		if( isbadlock(ldesc) || pptr->locktype[ldesc] == DELETED ){
//		kprintf("my state of lock %d\n", lptr->lstate);
//		enable(ps);
//		return SYSERR;
//		}
//		kprintf("my state of lock %d\n", lptr->lstate);
//		if(lptr->lstate == DELETED || pptr->pwaitret == DELETED  ){
//		kprintf("my state of lock %d\n", lptr->lstate);	
//		enable(ps);
//		return DELETED;
//		}
//		kprintf("my state of lock %d\n", lptr->lstate);
//		 kprintf("I am in lock\n");

		if (isbadlock(ldesc) || (lptr= &locktab[ldesc])->lstate==LFREE) {
	              restore(ps);
        	        return(SYSERR);
	        }
		if ( proctab[currpid].locktype[ldesc] < 0){
			restore(ps);
			return SYSERR;
		}
		if ( lptr->lstate == DELETED ){
			restore(ps);
			return DELETED;
		} 	
//		kprintf("type of process requesting lock %d\n", type);
//		 kprintf("my state of lock %d\n", lptr->lstate);
		 kprintf("type of holding lock %d\n", lptr->ltype);

		if(lptr->ltype==NOT_TAKEN_LOCK){	
//		if(lptr->lstate==LFREE){
//			 kprintf("type of process requesting the lock %d\n", type);
			 kprintf("I am in lock in NOT_TAKEN_LOCK\n");
//			kprintf("state of the lock %d\n", lptr->lstate);
			if(type==READ){
//				kprintf("state of the lock %d\n", lptr->lstate);
				lptr->ltype=TAKEN_LOCK_R;
				pptr->locktype[ldesc]=ASSIGNED_R;
				kprintf(" lock assigned to first process  %s, %d", pptr->pname, pptr->locktype[ldesc]);
				lptr->nreaders=lptr->nreaders+1;
			}
			else if(type==WRITE){
				kprintf("am i always cpming here  %s", pptr->pname);
				kprintf("state of the lock%s", pptr->pname);
				locktab[ldesc].ltype=TAKEN_LOCK_W;
				pptr->locktype[ldesc]=ASSIGNED_W;			
			}
			locktab[ldesc].lstate=LUSED;
//			 kprintf("state of the lock %d\n", lptr->lstate);	
			restore(ps);
			return OK;
		}
		else {
//			kprintf("type of process currently holding lock %d\n", lptr->ltype );
//			kprintf("type of process requesting for lock %d\n", type);
//			kprintf("ltype for checking with type %s, %d \n",pptr->pname, lptr->ltype);
			if(lptr->ltype==TAKEN_LOCK_W){
				pptr=&proctab[currpid];
				pptr->locktype[ldesc] = type;
				pptr->pstate = PRWWAIT;
				pptr->lprio = priority;
				pptr->wait_time = ctr1000;
 /* priority ramp up if the process holding lock has less priority than the process requesting it */
				insert(currpid, lptr->lqhead, priority); /* put the process in wait queue */
				resched();
				restore(ps);
				return (OK);
/*	
//				 kprintf("type of process currently holding lock\n");
				pptr->pstate=PRWWAIT;
				 kprintf("hi i am here when TAKEN_LOCK_W %s, %d\n", pptr->pname, type);
//				kprintf("type of process currently holding lock %s, %d\n", pptr->pname, lptr->ltype);
				if(type==READ)
					pptr->locktype[ldesc]=WAIT_R;
				else
					pptr->locktype[ldesc]=WAIT_W;
				proctab[currpid].wait_time=ctr1000;
//				kprintf("type of process requesting lock %d\n", proctab[currpid].locktype[ldesc]);
				proctab[currpid].lprio=priority;
				insert(currpid, lptr->lqhead, priority);
//				 kprintf("putting in waiting queue %s, %d \n",pptr->pname,  proctab[currpid].locktype[ldesc]); 
				pptr->pwaitret = OK;
				resched();
				restore(ps);
                		return pptr->pwaitret;
*/
			}
			else if(lptr->ltype==TAKEN_LOCK_R){
				kprintf("I am in TAKEN LOCK R *************************\n");
//				kprintf("lock is assigned to%s, %d \n",pptr->pname,  proctab[currpid].locktype[ldesc]);
				if(type==WRITE){
//					kprintf("state of the lock%s", pptr->pname);
					kprintf("hi i am here when type == WRITE %s, %d\n", pptr->pname, type);
					pptr->pstate=PRWWAIT;
					pptr->locktype[ldesc]=WAIT_W;
					kprintf("put lock to waiting %s, %d \n",pptr->pname,  pptr->locktype[ldesc]);
					proctab[currpid].wait_time=ctr1000;
					pptr->lprio=priority;	
					insert(currpid, lptr->lqhead,priority);
					pptr->pwaitret = OK;
					resched();
					restore(ps);
					return pptr->pwaitret;
				}
				else{
//				else if(type==READ){
					kprintf("hi i am here when type == READ %s, %d\n", pptr->pname, type);
//					kprintf("i am reader after writer %s, %d \n",pptr->pname,  proctab[currpid].locktype[ldesc]);
					highest_prio=checkforprio(ldesc);
					if(lptr->lqtail==highest_prio && (q[highest_prio].qkey>priority))
					{
						kprintf("when higher priority write is present %s \n",pptr->pname);
						dequeue(highest_prio);
						ready(highest_prio, RESCHNO);
						proctab[currpid].lprio=priority;
						proctab[currpid].wait_time=ctr1000;
						insert(currpid, lptr->lqhead, priority);
						proctab[currpid].pwaitret = OK;
						restore(ps);
						return proctab[currpid].pwaitret;
					}else if(q[highest_prio].qkey==priority){
						kprintf("hi i am here when priority of both are equal %s\n", pptr->pname);
						 kprintf("hi i am here when type == WRITE %s, %d\n", pptr->pname, type);
						if(ctr1000-proctab[highest_prio].wait_time>50){
							dequeue(highest_prio);
							ready(highest_prio, RESCHNO);
							proctab[currpid].lprio=priority;
							proctab[currpid].wait_time=ctr1000;
							insert(currpid, lptr->lqhead,priority);
							proctab[currpid].pwaitret = OK;
							restore(ps);
							return proctab[currpid].pwaitret;
						}
						else{
							kprintf("hi i  %s, %d\n", pptr->pname, type);
							index=q[lptr->lqtail].qprev;
							while(index!=highest_prio){
								locktab[ldesc].ltype=TAKEN_LOCK_R;
								lptr->nreaders=lptr->nreaders+1;
								proctab[index].locktype[ldesc]=ASSIGNED_R;
								index=q[index].qprev;
							}		
							restore(ps);
											
						}
					}
					else{
						kprintf("hi i am here when type == WRITE %s, %d\n", pptr->pname, type);
						kprintf("hi i am here when no higher prio writer is present %s \n ", pptr->pname);
						index=q[lptr->lqtail].qprev;
						while(index!=highest_prio){
							if(	
								pptr->locktype[ldesc]=WAIT_R){
								locktab[ldesc].ltype=TAKEN_LOCK_R;
								lptr->nreaders=lptr->nreaders+1;
								proctab[index].locktype[ldesc]=ASSIGNED_R;
							}
							index=q[index].qprev;
						}
						lptr->lstate = LUSED;
						restore(ps);
						return OK;
					}
				}
				
			}				
		}					
}			
							
							
int getlid(int i) {
    int lid=0;
    struct lentry *lptr;
    if (i > 0) {
        lptr = &locktab[i];
        while(i<NLOCKS){
            if(lptr->ldesc==i){
                lid=i;
            }
        i++;
        }
    return lid;
        }
}

int checkforprio(int ldesc) {
        struct lentry *lptr;
        struct pentry *pptr;
        int maxprio=0;
        int permission=0;
        lptr = &locktab[ldesc];
        int index = q[lptr->lqtail].qprev;
        while (index != lptr->lqhead) {
                pptr = &proctab[index];
                if (proctab[index].locktype[ldesc] == WAIT_W) {
                        return index;
                }
                index = q[index].qprev;
        }
        return -1;
}
	
						
				





/*

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>


extern long ctr1000;
extern int permission;

int getlid(int i) {
    int lid=0;
    struct lentry *lptr;
    if (i > 0) {
        lptr = &locktab[i];
        while(i<NLOCKS){
            if(lptr->ldesc==i){
                lid=i;
            }
        i++;
        }
    return lid;
	}
}
int checkforprio(int ldesc, int priority) {
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

int maxprio_sched(int ldesc){
	struct lentry *lptr;
	struct pentry *pptr;
	int maxprio=0;
	lptr=&locktab[ldesc];
	int index=q[lptr->lqhead].qnext;
	pptr=&proctab[index];
	while(index!=lptr->lqtail){
		if(maxprio<pptr->pprio && pptr->pstate==PRWWAIT){
			maxprio=pptr->pprio;
		}
	index=q[index].qnext;
	}
}

int lock (int ldesc, int type, int priority) {

	STATWORD ps;
	struct lentry *lptr;
        struct pentry *pptr;
        pptr = &proctab[currpid];
        lptr = &locktab[ldesc];
        int lid=getlid(ldesc);
	int highest_prio=0;
		disable(ps);
	int maxprio  = maxprio_sched(ldesc);
	if (isbadlock(ldesc) || (lptr= &locktab[ldesc])->lstate==LFREE || proctab[currpid].locktype[ldesc] < 0) {
                restore(ps);
                return(SYSERR);
        }
	if ( lptr->lstate == DELETED ){
		restore(ps);
		return DELETED;
	}	
	if(lptr->ltype==TAKEN_LOCK){
		if(type==WRITE){
			if(maxprio<proctab[currpid].pprio)
				priorityInheritance(proctab[currpid].pprio, ldesc);
			proctab[currpid].pstate = PRWWAIT;
			proctab[currpid].locktype[ldesc]=WAIT_W;
			insert(currpid, lptr->lqhead, pptr->lprio);
                        proctab[currpid].wait_time=ctr1000;
                        pptr->pwaitret = OK;
                        resched();
                        restore(ps);
	                return pptr->pwaitret;
		}	
		if(lptr->ltype==TAKEN_LOCK_R){
			pptr=&proctab[currpid];
			highest_prio=checkforprio(ldesc, priority);
			if(permission==1){
				if(proctab[highest_prio].lprio>pptr->lprio){
					proctab[currpid].pstate=PRWWAIT;
					proctab[currpid].wait_time=ctr1000;
					if(maxprio<proctab[currpid].pprio)
                                       		priorityInheritance(proctab[currpid].pprio, ldesc);
					proctab[currpid].locktype[ldesc]=WAIT_R;
					insert(currpid, lptr->lqhead, pptr->lprio);
					pptr->pwaitret = OK;
					resched();
					restore(ps);
       				       	return pptr->pwaitret;
		
				}else if(proctab[highest_prio].lprio==pptr->lprio){
					if(ctr1000-proctab[highest_prio].wait_time>500){
						proctab[currpid].pstate=PRWWAIT;
						proctab[currpid].wait_time=ctr1000;
				 		if(maxprio<proctab[currpid].pprio)
                                               		priorityInheritance(proctab[currpid].pprio, ldesc);
                                       		proctab[currpid].locktype[ldesc]=WAIT_R;
                                       		insert(currpid, lptr->lqhead, pptr->lprio);
						pptr->pwaitret = OK;
						resched();
						restore(ps);
						return pptr->pwaitret;
					}
				}else{
					lptr->ltype=TAKEN_LOCK_R;
					pptr->locktype[ldesc]=ASSIGNED_R;
					lptr->nreaders++;
					locktab[ldesc].lstate=LUSED;
				}
			} else{
				proctab[currpid].locktype[ldesc]=WAIT_R;
				insert(currpid, lptr->lqhead, pptr->lprio);
                                pptr->pwaitret = OK;
                                resched();
                                restore(ps);
                                return pptr->pwaitret;
                         }			
		}else if (lptr->ltype=TAKEN_LOCK_W)	
		{	
			highest_prio=checkforprio(ldesc, priority);
                 	pptr=&proctab[currpid];
			proctab[currpid].pstate=PRWWAIT;
			proctab[currpid].locktype[ldesc]=WAIT_R;
			proctab[currpid].wait_time=ctr1000;
			if(maxprio<proctab[currpid].pprio)
                                priorityInheritance(proctab[currpid].pprio, ldesc);
                                if(permission == 1){
					if(proctab[highest_prio].lprio==proctab[currpid].lprio){
						if(ctr1000-proctab[highest_prio].lprio>500){
							insert(currpid, lptr->lqhead, pptr->lprio);
						}
					}
					else {
					//add the next highest writer
					insertafterwriter(currpid, highest_prio,ldesc);
					}
                                        pptr->pwaitret = OK;
                                        resched();
                                        restore(ps);
                                        return pptr->pwaitret;
						
				}else{
					insert(currpid, lptr->lqhead, pptr->lprio);
					pptr->pwaitret = OK;
					resched();
					restore(ps);
               				return pptr->pwaitret;
				}
											
		}
	}else if(locktab[ldesc].lstate==LFREE){
		if(type==READ){
			lptr->ltype=TAKEN_LOCK_R;
			pptr->locktype[ldesc]=4;
			lptr->nreaders++;
		}
		else if(type==WRITE){
			locktab[ldesc].ltype=TAKEN_LOCK_W;
			pptr->locktype[ldesc]=3;		
			locktab[ldesc].nwriters++;	
		}
		locktab[ldesc].lstate=LUSED;		
		restore(ps);
		return OK;
		}
}		

void priorityInheritance(int priority, int ldesc){
	int pid=0;
	struct pentry *proc;
	while(pid<NPROC){
		proc=&proctab[pid];
		if(pid!=currpid || proc->locktype[ldesc]<=0){
			if(proc->locktype[ldesc]==ASSIGNED_W || proc->locktype[ldesc]==ASSIGNED_R){
			proc->pinh=priority;
			pid++;
			}
		}else{
			pid++;
			continue;
		}
	}
}


void insertafterwriter( int pid , struct pentry *optr, int ldesc){
	struct lentry *lptr = &locktab[ldesc];
	int index = lptr->lqtail;
	while( &proctab[index] != optr ){
		index = q[index].qprev;
	} 
	q[pid].qnext = q[index].qnext;
	q[pid].qprev = index;
	q[index].qnext = pid;
	q[q[pid].qnext].qprev = pid;
}
			 
		
			
		
		
		






int lock (int ldesc, int type, int priority) {
	STATWORD ps;
	struct lockentry *lptr;
	struct pentry *pptr;
	pptr = &proctab[currpid];
	lptr = &locktab[ldesc];
	int lid=getlid(ldesc);
	disable(ps);

	int checkpriority(int ldesc, int priority){
		lptr=&locktab[ldesc];
		int index=lptr->lqhead;
		int  permission = 0;
		while(q[index].qnext!=lptr->lqtail){
			if((proctab[q[index].qnext].waitpprio>priority) && (proctab[q[index].qnext.locktype[ldesc]]==WRITE )){
				permission = 1;
			}
		index = q[index].qnext;
		}
	return permission;
	}

	int getlid(int i) {
	int lid;
	if (i > 0) {
		lptr = &locktab[i];
		while(i<NLOCKS){
			if(lptr->ldesc==i){
				lid=i;
			}
		i++;
		}
	return lid;
	}
	return SYSERR;
	}
			}
	if (isbadlock(lid) || type == DELETED) {
		restore(ps);
		return SYSERR;
	}
	
	if(locktab[ldesc].lstate==LFREE){
		pptr->locktype[lid] = type;
		pptr->waitpprio = priority;
		lptr->lstate=LUSED;
		lptr->ltype=type;
		pptr->lid=-1;
		lptr->prochold[currpid]=1;
		lptr->lprio=pptr->pprio;
		restore(ps);
		return OK;
	}else if(lptr->lstate!=LFREE){
		if(lptr->ltype==WRITE){
			pptr->pstate=PRWAIT;
			pptr->locktype[lid]=type;
			pptr->waitpprio=priority;
			pptr->lid=ldesc;
			pptr->wait_time=ctr1000;
			increase_prio(lid, pptr->pprio);
			insert(currpid, lptr->lqhead, priority);
			resched();
			restore(ps);
			return OK;
		}else if(lptr->ltype==READ){
			if(type==READ){
				int trigger = checkpriority(int ldesc, int priority);
				if(trigger!=1){
					pptr->waitpprio=priority;
					lptr->ltype->type;
					lptr->prochold[currpid]=1;
					pptr->locktype[lid]=type;
					if(pptr->pprio>lptr->lprio){
						lptr->lprio=pptr->pprio;
					}				
				restore(ps);
				return OK;
				} else if (trigger==1){
					pptr->locktype[lid]=type;
					pptr->pstate=PRWAIT;
					pptr->lid=ldesc;
					pptr->waitpprio=priority;
					pptr->wait_time=ctr1000;
					increase_prio(lid,pptr->pprio);
					insert(currpid, lptr->lqhead, priority);
					resched();
					restore(ps);
					return (OK);
				}
			}else if(type==WRITE){
				pptr->locktype[lid] = type;
				pptr->pstate = PRWAIT;
				pptr->lid = ldesc;
				pptr->waitpprio = priority;
				pptr->wait_time = ctr1000;
				priorityRampUp(ldesc, pptr->pprio);
				insert(currpid, lptr->lqhead, priority);
				resched();
				restore(ps);
				return (OK);
			}
		}
	}
					
				
			
			
					
*/			
