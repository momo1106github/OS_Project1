#define _GNU_SOURCE
#include <sched.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include<sys/stat.h>

#define MAX_PROCESS_NUM 1000
#define TIME_QUANTUM 500

#define PRIORITY_LOW 10
#define PRIORITY_HIGH 90

#define ERR_EXIT(a) { perror(a); exit(1); }

#define SET_CPU(pid, i) do { \
			cpu_set_t mask; \
			CPU_ZERO(&mask); \
			CPU_SET((i), &mask); \
			if (sched_setaffinity((pid), sizeof(mask), &mask) != 0) \
				ERR_EXIT("sched_setaffinity"); \
} while (0)

#define SET_PRIORITY(pid, policy, priority) do { \
			struct sched_param param; \
			param.sched_priority = priority; \
			if (sched_setscheduler(pid, policy, &param) != 0) \
			    ERR_EXIT("sched_setscheduler");	\
} while (0)

#define wait_one_unit() do { \
		volatile unsigned long i; \
		for(i=0;i<1000000UL;i++); \
} while(0)

typedef struct Process {
	int idx;
	char Name[32];
	int R, T;
	pid_t pid;
} Process;

int start_process(Process p);
