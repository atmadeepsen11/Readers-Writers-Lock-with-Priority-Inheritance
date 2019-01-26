#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include <lock.h>


extern unsigned long ctr1000;
extern int permission;

int getlockid(int ldesc){
	int lockid=0;
	if (ldesc > 0) {
		for (ldesc = 0; ldesc < NLOCKS; ldesc ++) {
			if (locktab[ldesc].ldesc == ldesc) {
				lockid = ldesc;
			}
		}
		return lockid;
	}
	return SYSERR;
}
	

int lock (int ldesc, int type, int priority) {
//	kprintf("--------------------stats here----------------- \n");
	STATWORD ps;
	struct lentry *lptr;
    	struct pentry *pptr;
	int procpid=0;
    	pptr = &proctab[currpid];
    	lptr = &locktab[ldesc];
 	int highest_prio=0;
	int index=0;
	int maxprio=0;
	disable(ps);
	
	int flag = 0;		
//	kprintf("-----------asking for lock prcess %s =----------------- \n", proctab[currpid].pname);
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

//	kprintf("type of holding lock for newly created process %d, %s\n", lptr->ltype, pptr->pname);
	
	if(lptr->ltype == NOT_TAKEN_LOCK){	
//		kprintf("type of holding lock %d\n", lptr->ltype);
		if(type==READ){
			lptr->ltype = TAKEN_LOCK_R;
			pptr->locktype[ldesc] = ASSIGNED_R;
//			kprintf(" lock assigned to first process  %s, %d \n", pptr->pname, pptr->locktype[ldesc]);
			lptr->nreaders++;
		}
		else if(type==WRITE){
			lptr->nwriters++;
//			kprintf("state of the lock%s", pptr->pname);
			locktab[ldesc].ltype = TAKEN_LOCK_W;
			pptr->locktype[ldesc] = ASSIGNED_W;			
		}
		locktab[ldesc].lstate=LUSED;
		restore(ps);
		return OK;
	}
	else {
//		 kprintf("May be someone already has the lock so check if it write or read %d\n", lptr->ltype);
		if(lptr->ltype == TAKEN_LOCK_W){
			procpid = getpid();
			maxprio = get_maxprio(ldesc);
                        priority_inheritance(maxprio, ldesc);
			flag = 1;
		}
		else if(lptr->ltype==TAKEN_LOCK_R){
			if(type==WRITE){
				pptr->locktype[ldesc]=WAIT_W;
				maxprio = get_maxprio(ldesc);
                                priority_inheritance(maxprio, ldesc);
//				kprintf("state of the writers  %s, %d", pptr->pname, pptr->locktype[ldesc]);
				//    readers allready exists and write is asking for lock
				flag = 1;
			} 
			else{
			
//				 kprintf(" readers allready exist and new reader is asking for lock %s, %d \n", pptr->pname, pptr->pprio);
				if(!isempty(lptr->lqhead) ){
					int index = q[lptr->lqtail].qprev;
					while(index!=lptr->lqhead){
						if(priority < q[index].qkey){
							maxprio = get_maxprio(ldesc);
        						priority_inheritance(maxprio, ldesc);
							flag = 1;
						}
						else{
							lptr->nreaders++;
							lptr->ltype = TAKEN_LOCK_R;
                        				pptr->locktype[ldesc] = ASSIGNED_R;
//							kprintf("process %s got assigned the lock, %d  \n", proctab[currpid].pname, pptr->locktype[ldesc]);
						}
						index=q[index].qprev;
					}
				}
			}
		}
	}

	if(flag == 0){
		lptr->prochold[currpid]=1;
		pptr->lock_id=-1;
//		kprintf("state of the writers  %s, %d", pptr->pname, pptr->locktype[ldesc]);
//		kprintf("process %s got assigned the lock, %d  \n", proctab[currpid].pname, pptr->locktype[ldesc]);
//		kprintf("----------------------------------------------------------");		
		enable(ps);
		return OK;
	}
	else{
        insert(currpid,lptr->lqhead,priority);
	pptr->lock_id = ldesc;
//	kprintf("state of the writers  %s, %d \n", pptr->pname, pptr->locktype[ldesc]);
	pptr->wait_time = ctr1000;
	pptr->plwaitret = OK;
	pptr->pstate = PRWWAIT;
//	kprintf("process %s got blocked \n",proctab[currpid]);
//	kprintf("-------------------------------------------------");
	maxprio = get_maxprio(ldesc);
	priority_inheritance(maxprio, ldesc);
	int temp =q[lptr->lqhead].qnext;
//	kprintf("head %d   tail %d\n", q[lptr->lqhead].qnext, q[q[lptr->lqtail].qprev].qprev);
//	kprintf("Printing list of lock \n");
	while(temp != lptr->lqtail){
		temp = q[temp].qnext;
//		kprintf(" value is : %d \n", temp);
	}
//	kprintf("---------------------------------------------------");	
	resched();
	enable(ps);
	return OK;
}

}


void priority_inheritance( int priority , int ldesc){
	int i=0,proc_state;
	struct pentry *pptr;
	struct lentry *lptr;
	for( i = 0 ; i < NPROC ; i++ ){
		if( i == currpid )
			continue;
		if ( proctab[i].locktype[ldesc] <= 0 )
			continue;
		
		proc_state = proctab[i].locktype[ldesc];
		if ( proc_state == ASSIGNED_R  || proc_state == ASSIGNED_W){
			proctab[i].pinh = priority;
		}
//		if(pptr->lock_id!=-1)
//			priority_inheritance(priority , ldesc);
	}	
}

int get_maxprio(int ldesc){
	int maxprio = 0;
	struct lentry *lptr;
	struct pentry *pptr;
	int head = locktab[ldesc].lqhead;
	int tail = locktab[ldesc].lqtail;
	int index = q[tail].qprev;
	while ( index != head){
		if ( proctab[index].pstate == PRWWAIT && proctab[index].pprio > maxprio ){
			maxprio = proctab[index].pprio;
		}
		index = q[index].qprev;
	}
	return maxprio;
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
