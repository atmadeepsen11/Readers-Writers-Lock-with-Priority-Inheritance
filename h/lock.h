#ifndef _LOCK_H_
#define _LOCK_H_

#ifndef NLOCKS
#define NLOCKS	50
#endif

#define READ   0
#define WRITE  1

#define	LFREE	'\01' 
#define	LUSED	'\02'
#define DELETED	 '\00'

#define WAIT_W		1
#define WAIT_R		2
#define ASSIGNED_W	3
#define ASSIGNED_R	4

#define NOT_TAKEN_LOCK	0
#define TAKEN_LOCK_R	1
#define TAKEN_LOCK_W	2

#define isbadlock(l) (l<0 || l>=NLOCKS)

struct lentry {
	char	lstate;
	int	locknum;
	int	lqhead;
	int 	lqtail;
	int	nreaders;
	int 	nwriters;
	int	ltype;
	int	ldesc;
	int	maxlprio;
	int 	prochold[NPROC];
	
};

extern struct lentry locktab[];
extern int nextlock;
extern int permission;
//extern int nextldesc;
int checkforprio(ldesc);

void linit();
int lcreate();
int ldelete(int lockdescriptor);
int lock(int ldes1, int type, int priority);
int releaseall(int numlocks, int args,...);
int releaselock(int pid, int ldesc, int toggle);
extern int getlockid(int ldesc);
extern void testingsem();
extern void testinglock();

#endif
