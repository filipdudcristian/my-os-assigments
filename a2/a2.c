#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "a2_helper.h"
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>

typedef struct
{
    int pid;
    int tid;
    pthread_mutex_t *lock;
    pthread_cond_t *cond;
    pthread_t *threads;

} TH_STRUCT;

int nrThreads = 0;

sem_t *semPb5_1, *semPb5_2, *sem15, *semstop;
int x = 0;

void *thread_function_pb2(void *arg)
{
    TH_STRUCT *thread = (TH_STRUCT *)arg;

    if (thread->tid == 4)
        pthread_cond_wait(thread->cond, thread->lock);

    info(BEGIN, thread->pid, thread->tid);

    if (thread->tid == 1)
    {
        pthread_cond_signal(thread->cond);
        pthread_join(thread->threads[4], NULL);
    }

    info(END, thread->pid, thread->tid);

    return NULL;
}

void *thread_function_pb3(void *arg)
{
    // TH_STRUCT *thread = (TH_STRUCT *)arg;

    // if (contor >= 4)
    //     pthread_cond_wait(thread->cond, thread->lock);

    // info(BEGIN, thread->pid, thread->tid);
    // contor++;

    // info(END, thread->pid, thread->tid);

    TH_STRUCT* th_p = (TH_STRUCT*)arg;

    // if (th_p->tid == 15)
	// {
	// 	sem_wait(sem15);
	// }

	pthread_mutex_lock(th_p->lock);
	x++;
	while (x > 4)
        pthread_cond_wait(th_p->cond, th_p->lock);
	if(x < 4){ //&& th_p->tid != 13){
		
		//sem_wait(semPb5_1);
		info(BEGIN, th_p->pid, th_p->tid);
        x--;
        pthread_cond_signal(th_p->cond);
		info(END, th_p->pid, th_p->tid);
		//sem_post(semPb5_1);
	}

    pthread_mutex_unlock(th_p->lock);
	// else
	// {
		
	// 	sem_wait(semPb5_1);
	// 	info(BEGIN, th_p->pid, th_p->tid);
	// 	if (x==40)
	// 	{
	// 		sem_post(sem15);
	// 	}
	// 	pthread_mutex_unlock(th_p->lock);
	// 	if (th_p->tid != 13)
	// 	{
	// 		sem_wait(sem);
	// 		info(END, th_p->pid, th_p->tid);
	// 		sem_post(semPb5_1);
	// 	}
	// 	else
	// 	{
	// 		info(END, th_p->pid, th_p->tid);
	// 		sem_post(semPb5_1);
	// 		sem_post(semstop);
	// 		sem_post(semstop);
	// 		sem_post(semstop);
	// 		sem_post(semstop);
	// 	}
	// }

    return NULL;
}





int main()
{
    init();
    pid_t pid1 = -1, pid2 = -1, pid3 = -1, pid4 = -1, pid5 = -1, pid6 = -1, pid7 = -1, pid8 = -1;
    semPb5_1 = sem_open("semPb5_1", O_CREAT, 0644, 0);
	semPb5_2 = sem_open("semPb5_1", O_CREAT, 0644, 0);
    sem15 = sem_open("sem15", O_CREAT, 0644, 0);
	semstop = sem_open("semstop", O_CREAT, 0644, 0);

    info(BEGIN, 1, 0);
    pid1 = getpid();
    pid2 = fork();
    if (pid2 == 0)
    {
        info(BEGIN, 2, 0);

        pid4 = fork();
        if (pid4 == 0)
        {

            info(BEGIN, 4, 0);

            pid5 = fork();
            if (pid5 == 0)
            {
                info(BEGIN, 5, 0);

                pid6 = fork();
                if (pid6 == 0)
                {
                    info(BEGIN, 6, 0);

                    info(END, 6, 0);
                    exit(6);
                }

                waitpid(pid6, NULL, 0);
                info(END, 5, 0);
                exit(5);
            }

            pthread_t threads[41];
            TH_STRUCT params[41];

            pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
            pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

            for (int i = 1; i <= 40; i++)
            {
                params[i].pid = 4;
                params[i].tid = i;
                params[i].lock = &lock;
                params[i].cond = &cond;
                params[i].threads = threads;

                pthread_create(&threads[i], NULL, thread_function_pb3, &params[i]);
            }

            for (int i = 1; i <= 40; i++)
            {
                pthread_join(threads[i], NULL);
            }

            pthread_mutex_destroy(&lock);
            pthread_cond_destroy(&cond);

            waitpid(pid5, NULL, 0);
            info(END, 4, 0);
            exit(4);
        }

        pthread_t threads[5];
        TH_STRUCT params[5];

        pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
        pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

        for (int i = 1; i <= 4; i++)
        {
            params[i].pid = 2;
            params[i].tid = i;
            params[i].lock = &lock;
            params[i].cond = &cond;
            params[i].threads = threads;

            pthread_create(&threads[i], NULL, thread_function_pb2, &params[i]);
        }

        for (int i = 1; i <= 4; i++)
        {
            pthread_join(threads[i], NULL);
        }

        pthread_mutex_destroy(&lock);
        pthread_cond_destroy(&cond);

        waitpid(pid4, NULL, 0);
        info(END, 2, 0);
        exit(2);
    }

    if (getpid() == pid1)
    {
        pid3 = fork();
        if (pid3 == 0)
        {
            info(BEGIN, 3, 0);

            pid7 = fork();
            if (pid7 == 0)
            {
                info(BEGIN, 7, 0);

                info(END, 7, 0);
                exit(7);
            }

            waitpid(pid7, NULL, 0);
            info(END, 3, 0);
            exit(3);
        }
    }

    if (getpid() == pid1)
    {
        pid8 = fork();
        if (pid8 == 0)
        {
            info(BEGIN, 8, 0);

            info(END, 8, 0);

            pthread_t threads[7];
            TH_STRUCT params[7];

            pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
            pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

            for (int i = 1; i <= 6; i++)
            {
                params[i].pid = 8;
                params[i].tid = i;
                params[i].lock = &lock;
                params[i].cond = &cond;
                params[i].threads = threads;

                pthread_create(&threads[i], NULL, thread_function_pb2, &params[i]);
            }

            for (int i = 1; i <= 7; i++)
            {
                pthread_join(threads[i], NULL);
            }

            pthread_mutex_destroy(&lock);
            pthread_cond_destroy(&cond);

            exit(8);
        }
    }

    waitpid(pid2, NULL, 0);
    waitpid(pid3, NULL, 0);
    waitpid(pid8, NULL, 0);

    info(END, 1, 0);
    return 0;
}