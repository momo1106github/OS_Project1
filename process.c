#include "process.h"
#include "scheduler.h"

extern int *end_flag;

int start_process(Process p)
{
	int pid;
	if ((pid = fork()) < 0) {
		ERR_EXIT("fork");
	} else if (pid == 0) {	
		printf("%s %d\n", p.Name, getpid());
		long start_time = syscall(335);
		//for (int i = 0; i < p.T; i++)
		while (*end_flag == 0)
			wait_one_unit();
		long end_time = syscall(335);
		syscall(334, pid, start_time, end_time);
		static const long BASE = 1000000000;
		fprintf(stderr, "[Project1] %d %ld.%09ld %ld.%09ld\n", getpid(), \
				start_time / BASE, start_time % BASE, end_time / BASE, end_time % BASE);
		exit(0);
	} else { 
		SET_PRIORITY(pid, SCHED_FIFO, PRIORITY_LOW);
		SET_CPU(pid, 1);
	}
	return pid;
}
