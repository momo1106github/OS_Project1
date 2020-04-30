#define _GNU_SOURCE

#include "scheduler.h"
#include "shared.h"

extern int nProcess;
extern Process P[MAX_PROCESS_NUM];

int cmp(const void* a, const void* b) {
	Process p1 = *(Process *)a, p2 = *(Process *)b;
	if (p1.R == p2.R) return p1.idx > p2.idx;
	return p1.R > p2.R;
}
int cmp1(const void* a, const void* b) {
	Process p1 = *(Process *)a, p2 = *(Process *)b;
	if (p1.R == p2.R && p1.T == p2.T) 
		return p1.idx > p2.idx;
	else if (p1.R == p2.R) 
		return p1.T > p2.T;
	else return p1.R > p2.R;
}

/* queue */
#define MAX_QUEUE_SIZE (int)1e6 + 5
int que[MAX_QUEUE_SIZE], front, back;

void queue_push(int x) { que[back++] = x; }
void queue_pop() { if (front < back) front++; }
int queue_empty() { return front == back; }
int queue_size() { return back - front; }
void print_queue() { fprintf(stderr, "in Queue: "); for (int i = front; i < back; i++) fprintf(stderr, "%d ", que[i]); fprintf(stderr, "\n");}

int *end_flag;

int nFinish;
int running_id = -1;
int clock_time, last_start_time;

#define FIFO 0
#define RR 1
#define SJF 2
#define PSJF 3

int find_next(int policy)
{	
	if (running_id != -1 && (policy == FIFO || policy == SJF)) return running_id;

	int min_id = -1;
	if (policy == FIFO) {
		for (int i = 0; i < nProcess; i++)
			if (P[i].pid && P[i].T > 0)
				if (min_id == -1 || P[i].R < P[min_id].R)
					min_id = i;
	} else if (policy == RR) {
		if (running_id == -1 && !queue_empty()) {
			min_id = que[front];
			queue_pop();
			//print_queue();
		} else if (clock_time != last_start_time && (clock_time - last_start_time) % TIME_QUANTUM == 0) {
			if (!queue_empty()) {
				min_id = que[front];
				queue_pop();
				if (P[running_id].T > 0) queue_push(running_id);
				//print_queue();
			}
		} else {
			min_id = running_id;
		}
	} else if (policy == SJF || policy == PSJF) {
		for (int i = 0; i < nProcess; i++)
			if (P[i].pid && P[i].T > 0)
				if (min_id == -1 || P[i].T < P[min_id].T)
					min_id = i;
	}
	return min_id;
}

int start_process(Process p)
{
	int pid;
	if ((pid = fork()) < 0) {
		ERR_EXIT("fork");
	} else if (pid == 0) {	
		int now_pid = getpid();
		printf("%s %d\n", p.Name, now_pid);
		long start_time = syscall(335);
		//for (int i = 0; i < p.T; i++)
		while (*end_flag == 0) {
			wait_one_unit();
			//*end_flag = 9999;
			//fprintf(stderr,"---end_flag at %d = %d\n", getpid(), *end_flag);
		}
		*end_flag = 0;
		//fprintf(stderr, "end_flag back to %d\n", *end_flag);
		long end_time = syscall(335);
		//fprintf(stderr, "syscall here\n");
		syscall(334, now_pid, start_time, end_time);
		static const long BASE = 1000000000;
		fprintf(stderr, "[Project1] %d %ld.%09ld %ld.%09ld\n", now_pid, \
			start_time / BASE, start_time % BASE, end_time / BASE, end_time % BASE);
		exit(0);
	} else { 
		SET_PRIORITY(pid, SCHED_FIFO, PRIORITY_LOW); //***order is important*** 
		SET_CPU(pid, 1);
	} 
	return pid;																								
}

void scheduler(int policy)
{
	if (policy == FIFO || policy == RR) qsort(P, nProcess, sizeof(P[0]), cmp);
	else if (policy == SJF || policy == PSJF) qsort(P, nProcess, sizeof(P[0]), cmp1);

	for (int i = 0; i < nProcess; i++)
		fprintf(stderr, "%s %d %d %d\n", P[i].Name, P[i].R, P[i].T, P[i].pid);
	
	/* share memory */
	const int shareSize = sizeof(int);
	int segmentId = shmget(IPC_PRIVATE, shareSize, S_IRUSR | S_IWUSR);
	end_flag = (int*)shmat(segmentId, NULL, 0);
	*end_flag = 0;

	int ready_id = 0;
	while (1) {
		/* Check if running process is finished */
		if (running_id != -1 && P[running_id].T == 0) {
			*end_flag = 1;
			//fprintf(stderr, "%s terminated at %d, finish = %d\n", P[running_id].Name, clock_time, nFinish);
			//wait(NULL);
			while (*end_flag == 1);//********don't add -O2 when compiling
				//fprintf(stderr, "end_flag = %d\n", *end_flag);		
			waitpid(P[running_id].pid, NULL, 0);
			P[running_id].pid = 0;
			running_id = -1;
			if (++nFinish == nProcess) break;
		}
		/* Check if any process are ready */
		while (ready_id < nProcess && P[ready_id].R == clock_time) {
			P[ready_id].pid = start_process(P[ready_id]);
			if (policy == RR) queue_push(ready_id);
		

			ready_id++;
			//fprintf(stderr, "%s is forked, pid = %d\n", P[ready_id-1].Name, P[ready_id-1].pid);
		}

		/* Find next process to be run */
		int next_id = find_next(policy);
		//if (clock_time % 100 == 0) fprintf(stderr, "%d : running_id %d, next_id %d\n", clock_time, running_id, next_id);
		if (next_id != -1 && next_id != running_id) {
		//	fprintf(stderr, "%d : running_id %s, next_id %s\n", clock_time, P[running_id].Name, P[next_id].Name);
			SET_PRIORITY(P[next_id].pid, SCHED_FIFO, PRIORITY_HIGH);
			if (running_id != -1) SET_PRIORITY(P[running_id].pid, SCHED_FIFO, PRIORITY_LOW);
			running_id = next_id;
			last_start_time = clock_time;
		}
				
		wait_one_unit();
		if (running_id != -1) P[running_id].T--;
		//if (P[running_id].T == 0) fprintf(stderr, "!!!! %s\n", P[running_id].Name);
		clock_time++;
	}
	//while (wait(NULL));
}


