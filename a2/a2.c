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
int found = 0;
int ended = 0;

sem_t *semPb2_1, *semPb2_2, *semPb3, *semPb4_1, *semPb4_2, *sem13, *semCount;
int count = 0;
int startedThreads = 0;

void *thread_function_pb2(void *arg)
{
    TH_STRUCT *thread = (TH_STRUCT *)arg;

    if (thread->pid == 2 && thread->tid == 4)
        sem_wait(semPb2_1);

    if (thread->pid == 2 && thread->tid == 3)
        sem_wait(semPb4_1);

    if (thread->pid == 8 && thread->tid == 6)
        sem_wait(semPb4_2);

    info(BEGIN, thread->pid, thread->tid);

    if (thread->pid == 2 && thread->tid == 1)
    {
        sem_post(semPb2_1);
        sem_wait(semPb2_2);
    }

    info(END, thread->pid, thread->tid);

    if (thread->pid == 2 && thread->tid == 4)
        sem_post(semPb2_2);

    ////////////////////////////////////////////////////
    if (thread->pid == 8 && thread->tid == 3)
        sem_post(semPb4_1);

    if (thread->pid == 2 && thread->tid == 3)
        sem_post(semPb4_2);

    return NULL;
}

void *thread_function_pb3(void *arg)
{
    TH_STRUCT *thread = (TH_STRUCT *)arg;

    if (thread->tid == 13)
    {
        sem_wait(sem13);
    }

    pthread_mutex_lock(thread->lock);
    startedThreads++;
    pthread_mutex_unlock(thread->lock);

    if (startedThreads >= 37 && thread->tid != 13)
    {

        sem_wait(semPb3);
        info(BEGIN, thread->pid, thread->tid);
        if (startedThreads == 39)
        {
            sem_post(sem13);
        }

        sem_wait(semCount);
        info(END, thread->pid, thread->tid);
        // sem_post(semPb3);
    }
    else
    {
        sem_wait(semPb3);
        info(BEGIN, thread->pid, thread->tid);

        if (thread->tid != 13)
        {

            info(END, thread->pid, thread->tid);
            // sem_post(semPb3);
        }
        else if (thread->tid == 13)
        {
            info(END, thread->pid, thread->tid);
            // sem_post(semPb3);
            for (int i = 0; i < 3; i++)
                sem_post(semCount);
        }
    }
    sem_post(semPb3);
    return NULL;
}

int main()
{

    init();
    sem_unlink("semPb3");
    sem_unlink("semPb4_1");
    sem_unlink("semPb4_2");
    sem_unlink("semPb5_2");
    sem_unlink("semPb2_1");
    sem_unlink("semPb2_2");
    sem_unlink("sem13");
    sem_unlink("semstop");
    
    semPb3 = sem_open("semPb3", O_CREAT, 0644, 4);
    semPb4_1 = sem_open("semPb4_1", O_CREAT, 0644, 0);
    semPb4_2 = sem_open("semPb4_2", O_CREAT, 0644, 0);
    semPb2_1 = sem_open("semPb2_1", O_CREAT, 0644, 0);
    semPb2_2 = sem_open("semPb2_2", O_CREAT, 0644, 0);
    sem13 = sem_open("sem13", O_CREAT, 0644, 0);
    semCount = sem_open("semCount", O_CREAT, 0644, 0);

    pid_t pid2 = -1, pid3 = -1, pid4 = -1, pid5 = -1, pid6 = -1, pid7 = -1, pid8 = -1;

    info(BEGIN, 1, 0);
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

        for (int i = 1; i <= 6; i++)
        {
            pthread_join(threads[i], NULL);
        }

        pthread_mutex_destroy(&lock);
        pthread_cond_destroy(&cond);

        exit(8);
    }

    waitpid(pid2, NULL, 0);
    waitpid(pid3, NULL, 0);
    waitpid(pid8, NULL, 0);

    info(END, 1, 0);
    return 0;
}