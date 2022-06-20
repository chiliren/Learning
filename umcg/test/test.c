#define _GNU_SOURCE
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>

#include <linux/types.h>

#ifndef __NR_umcg_ctl
#define __NR_umcg_ctl  450
#endif

#include "list.h"
#include "umcg.h"

/* syscall wrappers */

static inline int
sys_umcg_ctl(u32 flags, struct umcg_task *self, clockid_t which_clock)
{
	return syscall(__NR_umcg_ctl, flags, self, which_clock);
}

/* our workers */

static volatile bool done = false;

static void umcg_signal(int signr)
{
	done = true;
}

/* always running worker */
void *worker_fn0(void *arg)
{
	struct umcg_task *server = arg;
	struct umcg_task task = {};
	unsigned long i;
	int ret;

	task.server_tid = server->server_tid;
	task.state = UMCG_TASK_BLOCKED;

	printf("A == %d \n", gettid());

	ret = sys_umcg_ctl(UMCG_CTL_REGISTER|UMCG_CTL_WORKER, &task, 0);
	if (ret) {
		perror("umcg_ctl(A): ");
		exit(-1);
	}

	printf("A == start \n");

	while (!done) {
		int x = i++;

		if (!(x % 1000000)) {  /* 每循环一百万次就打印一个点 */
			putchar('.');
			printf("A runnable num: %lld\n", server->runnable_workers_sum);
			fflush(stdout);
		}
	}

	printf("A == done \n");
	ret = sys_umcg_ctl(UMCG_CTL_UNREGISTER|UMCG_CTL_WORKER, &task, 0);
	if (ret) {
		perror("umcg_ctl(~A): ");
		exit(-1);
	}

	return NULL;
}

/* event driven worker */
void *worker_fn1(void *arg)
{
	struct umcg_task *server = arg;
	struct umcg_task task = { };
	int ret;

	task.server_tid = server->server_tid;
	task.state = UMCG_TASK_BLOCKED;

	printf("B == %d \n", gettid());

	ret = sys_umcg_ctl(UMCG_CTL_REGISTER|UMCG_CTL_WORKER, &task, 0);
	if (ret) {
		perror("umcg_ctl(B): ");
		exit(-1);
	}


	printf("B == start \n");
	while (!done) {
		printf("B \n");
		fflush(stdout);

		sleep(1);  /* 每执行一次就 sleep 1s */
	}

	printf("B == done \n");


	ret = sys_umcg_ctl(UMCG_CTL_UNREGISTER|UMCG_CTL_WORKER, &task, 0);
	if (ret) {
		perror("umcg_ctl(~B): ");
		exit(-1);
	}

	return NULL;
}

/* the server */

int main(int argc, char **argv)
{
	struct umcg_task *runnable_ptr, *next;
	struct umcg_task server = { };
	pthread_t worker[8];
	u32 tid;
	int ret;

	struct sigaction sa = {
		.sa_handler = umcg_signal,
	};

	sigaction(SIGINT, &sa, NULL);

	printf("server == %d \n", gettid());
	fflush(stdout);

	server.server_tid = gettid();
	server.state = UMCG_TASK_RUNNING;

	ret = sys_umcg_ctl(UMCG_CTL_REGISTER, &server, 0);
	if (ret) {
		perror("umcg_ctl: ");
		exit(-1);
	}

	pthread_create(&worker[0], NULL, worker_fn0, &server);
	pthread_create(&worker[1], NULL, worker_fn1, &server);

	while(!done) {
		printf("Main: %lld\n", server.runnable_workers_sum);
		fflush(stdout);
	}

	pthread_join(worker[0], NULL);
	pthread_join(worker[1], NULL);

	return 0;
}
