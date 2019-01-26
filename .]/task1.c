#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <stdio.h>
#include <lock.h>

#define DEFAULT_LOCK_PRIO 20

#define assert(x,error) if(!(x)){ \
            kprintf(error);\
            return;\

void reader7 (char *msg, int lck)
{
        int     ret;

        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, READ, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock\n", msg);
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}

void writer7 (char *msg, int lck)
{
        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, WRITE, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock, sleep 10s\n", msg);
        sleep (5);
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}

void testinglock ()
{
        int     lck;
        int     rd1, rd2;
        int     wr1;

        kprintf("\n--------------------TESTING LOCK-------------------\n");
        lck  = lcreate ();
        assert (lck != SYSERR, "Lock Test failed");

        rd1 = create(reader7, 2000, 15, "reader3", 2, "reader A Lock", lck);
        rd2 = create(reader7, 2000, 20, "reader3", 2, "reader B Lock", lck);
        wr1 = create(writer7, 2000, 10, "writer3", 2, "writer Lock", lck);

        kprintf("-start writer, then sleep 1s. lock granted to write (prio 10)\n");
        resume(wr1);
        sleep (1);

        kprintf("-start reader A, then sleep 1s. reader A(prio 15) gets blocked\n");
        resume(rd1);
        sleep (1);
        kprintf("priority of writer:%d\n", getprio(wr1));

        kprintf("-start reader B, then sleep 1s. reader B(prio 20) gets blocked\n");
        resume (rd2);
        sleep (1);
        kprintf("priority of writer:%d\n", getprio(wr1));
    
        kprintf("-kill reader B, then sleep 1s\n");
        kill (rd2);
        sleep (1);
        kprintf("priority of writer:%d\n", getprio(wr1));

        kprintf("-kill reader A, then sleep 1s\n");
        kill (rd1);
        sleep(1);
        kprintf("priority of writer:%d\n", getprio(wr1));

        sleep (10);
}

int task1( )
{
	kprintf("Semaphore priority inversion\n");
	testingsem();
	kprintf("Locks priority inversion\n");
	testinglock();

        /* The hook to shutdown QEMU for process-like execution of XINU.
 *  *  *          * This API call exists the QEMU process.
 *   *   *                   */
        shutdown();
}



void reader8 (char *msg, int sem)
{
        kprintf ("  %s: to acquire sem\n", msg);
        wait(sem);
        kprintf ("  %s: acquired sem\n", msg);

        kprintf ("  %s: to release sem\n", msg);
        signal(sem);
}

void writer8 (char *msg, int sem)
{
        kprintf ("  %s: to acquire sem\n", msg);
        wait(sem);
        kprintf ("  %s: acquired sem, sleep 10s\n", msg);
        sleep (10);
        kprintf ("  %s: to release sem\n", msg);
        signal(sem);
}

void testingsem(){
        int     sem;
        int     rd1, rd2;
        int     wr1;

        kprintf("\n--------------SEMAPHORE TEST----------------------\n");
        sem  = screate (1);

        rd1 = create(reader8, 2000, 15, "reader3", 2, "reader 1 Sem", sem);
        rd2 = create(reader8, 2000, 20 , "reader3", 2, "reader 2 Sem", sem);
        wr1 = create(writer8, 2000, 10, "writer3", 2, "writer Sem", sem);

        kprintf("-start writer, then sleep 1s. sem granted to write (prio 10)\n");
        resume(wr1);
        sleep (1);

        kprintf("-start reader 1, then sleep 1s. reader 1(prio 15) gets blocked \n");
        resume(rd1);
        sleep (1);
        kprintf("priority of writer:%d\n", getprio(wr1));

        kprintf("-start reader 2, then sleep 1s. reader 2(prio 20) gets blocked\n");
        resume (rd2);
        sleep (1);
        kprintf("priority of writer:%d\n", getprio(wr1));
    
        kprintf("-kill reader 2, then sleep 1s\n");
        kill (rd2);
        sleep (1);
        kprintf("priority of writer:%d\n", getprio(wr1));

        kprintf("-kill reader 1, then sleep 1s\n");
        kill (rd1);
        sleep(1);
        kprintf("priority of writer:%d\n", getprio(wr1));

        sleep (10);
}
}
