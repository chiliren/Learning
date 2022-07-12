#define _GNU_SOURCE
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/prctl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>

#include <linux/types.h>

#include "wgcm.h"

#ifndef __NR_prctl
#define __NR_prctl 167
#endif

#ifndef PR_WGCM_CTL
#define PR_WGCM_CTL 59
#endif

#ifndef PR_GET_WGCM_TASK
#define PR_GET_WGCM_TASK 60
#endif

#define WORKER_NUM 8

struct client_args {
	int number;
	unsigned int server_tid;
};

/* cond */
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

/* mutex lock */
pthread_mutex_t client_mutex[WORKER_NUM];

static unsigned int flag = 0;

/* workers */
void *worker_fn(void *arg)
{
	struct client_args *args = (struct client_args *)arg;
	int number = args->number;
#ifdef CONFIG_WGCM
	unsigned int server_tid = args->server_tid;
	int ret;
#endif

#ifdef CONFIG_WGCM
	printf("worker[%d], server_tid = %u\n", number, server_tid);
	fflush(stdout);
	ret = prctl(PR_WGCM_CTL, WGCM_CTL_REGISTER|WGCM_CTL_WORKER, server_tid, 0, 0);
	if (ret) {
		perror("register worker failed\n");
		exit(-1);
	}
#endif

	printf("thread %d start!!!!! \n", number);
	fflush(stdout);

	while (1) {
		pthread_mutex_lock(&client_mutex[number]);

		if (flag & (1 << number)) {
			printf("[%d]thread %d start to wait \n", gettid(), number);
			pthread_cond_wait(&cond, &client_mutex[number]);
			printf("[%d]thread %d unblock \n", gettid(), number);
		}

		pthread_mutex_unlock(&client_mutex[number]);
	}

	printf("thread %d done!!!!! \n", number);
	fflush(stdout);
#ifdef CONFIG_WGCM
	ret = prctl(PR_WGCM_CTL, WGCM_CTL_UNREGISTER|WGCM_CTL_WORKER, server_tid);
	if (ret) {
		perror("unregister worker faild");
		exit(-1);
	}
#endif

}

/* the server */
int main(int argc, char **argv)
{
	pthread_t worker[WORKER_NUM];
	struct client_args args[WORKER_NUM];
	unsigned long i;
	unsigned int server_tid;

#ifdef CONFIG_WGCM
	int ret;
#endif

	printf("Main server == %d. \n", gettid());
	fflush(stdout);

	server_tid = gettid();

#ifdef CONFIG_WGCM
	ret = prctl(PR_WGCM_CTL, WGCM_CTL_REGISTER, server_tid, 0, 0);
	if (ret) {
		perror("[MAIN] wgcm_ctl failed");
		exit(-1);
	}
#endif

	for (i = 0; i < WORKER_NUM; i++) {
		pthread_mutex_init(&client_mutex[i], NULL);
		args[i].number = i;
		args[i].server_tid = server_tid;
		pthread_create(&worker[i], NULL, worker_fn, &args[i]);
	}

	sleep(1);

	int j = 1;
	while (1) {
		printf("test %d , please input flag(0~127):", j);
		scanf("%u", &flag);
		printf("\n");
		sleep(3);
#ifdef CONFIG_WGCM
		struct wgcm_task server;
		prctl(PR_GET_WGCM_TASK, (unsigned long)&server, 0, 0, 0);
		printf("[WGCM after] total_workers = %d, blk_workers = %d\n",
			server.workers_sum.counter, server.blk_workers_sum.counter);

#endif
		pthread_cond_broadcast(&cond);
		sleep(5);
		//assert
#ifdef CONFIG_WGCM
		prctl(PR_GET_WGCM_TASK, (unsigned long)&server, 0, 0, 0);
		printf("[WGCM after] total_workers = %d, blk_workers = %d\n",
			server.workers_sum.counter, server.blk_workers_sum.counter);
#endif
		j++;
		printf("\n");
	}

	for (i = 0; i < WORKER_NUM; i++)
		pthread_join(worker[i], NULL);

	return 0;
}
