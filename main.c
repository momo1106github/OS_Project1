#define _GNU_SOURCE

#include <assert.h>
#include "shared.h"
#include "scheduler.h"

int nProcess;
Process P[MAX_PROCESS_NUM];

int main(void)
{
	char S[10]; assert(scanf("%s", S) == 1); 
	assert(scanf("%d", &nProcess) == 1);
	for (int i = 0; i < nProcess; i++) {
		assert(scanf("%s%d%d", &P[i].Name, &P[i].R, &P[i].T) == 3);
		P[i].idx = i;
	}

	SET_CPU(getpid(), 0);
	SET_PRIORITY(getpid(), SCHED_FIFO, PRIORITY_HIGH);
	
	if (S[0] == 'F') scheduler(0);
	if (S[0] == 'R') scheduler(1);
	if (S[0] == 'S') scheduler(2);
	if (S[0] == 'P') scheduler(3);

	return 0;
}
