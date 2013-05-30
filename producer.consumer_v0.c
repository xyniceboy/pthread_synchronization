/*************************************************************************
  > File Name: pcm.c
	> Func:	Producer-consumer problem, an implementation of one producer,
		one consumer, mutiple buffer version.
	> Author: Ren Zhen
	> Mail: renzhengeek.com 
	> Created Time: 2013年05月22日 星期三 15时32分20秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

#define NBUFF 1024

int nitems;		/* read-only by producer and consumer */
struct {
	int buff[NBUFF];
	sem_t *mutex, *nempty, *nstored;	/* nempty stands for empty buff,
										   nstored for items to be consumed */
}shared;

void *produce(void *);
void *consume(void *);

int main(int argc, char *argv[])
{
	pthread_t tid_produce, tid_consume;

	if(argc != 2){
		fprintf(stderr, "Usage: pcm <items>\n");
		exit(EXIT_FAILURE);
	}

	nitems = atoi(argv[1]);

	shared.mutex = sem_open("mutex", O_CREAT | O_EXCL,
					0664, 1);
	if(shared.mutex == SEM_FAILED){
		perror("sem_open");
		exit(EXIT_FAILURE);
	}

	shared.nempty = sem_open("nempty", O_CREAT | O_EXCL,
			0664, NBUFF);

	if(shared.nempty == SEM_FAILED){
	 perror("sem_open");
	 exit(EXIT_FAILURE);
	}

	shared.nstored = sem_open("nstored", O_CREAT | O_EXCL,
			0664, 0);
	if(shared.nstored == SEM_FAILED){
		perror("sem_open");
		exit(EXIT_FAILURE);
	}

	pthread_create(&tid_produce, NULL, produce, NULL); /* Start a producer */
	pthread_create(&tid_consume, NULL, consume, NULL); /* Start a consumer */

	/* Wait for producer and consumer */
	pthread_join(tid_produce, NULL);	
	pthread_join(tid_consume, NULL);

	sem_unlink("mutex");
	sem_unlink("nempty");
	sem_unlink("nstored");

	exit(EXIT_SUCCESS);
}

void *produce(void *arg){
	int i;

	for(i = 0; i < nitems; i++){
		sem_wait(shared.mutex);
		sem_wait(shared.nempty);
		shared.buff[i % NBUFF] = i;
		printf("produce %d\n", i);
		sem_post(shared.nstored);
		sem_post(shared.mutex);
	}

	return (NULL);
}

void *consume(void *arg){
	int i;

	for(i = 0; i < nitems; i++){
		sem_wait(shared.mutex);
		sem_wait(shared.nstored);
		if(shared.buff[i%NBUFF] != i){
			printf("buf[%d] = %d\n", i, shared.buff[i%NBUFF]);
		}
		printf("consume %d\n", i);
		sem_post(shared.nempty);
		sem_post(shared.mutex);
	}

	return (NULL);
}
