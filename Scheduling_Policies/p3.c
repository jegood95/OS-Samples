/* jegood Joshua Good */

/**
 * @file p3.c
 * Schedules three processes based upon user specification: either FIFO, RR, or SCHED_OTHER.
 * Uses threads to run each process and prints their contents corresponding to the scheduling
 * policy. 
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <sched.h>
#include <stdint.h>

/** number of threads */
int NUM_THREADS = 3;
/** flag to start executing */
int start = 0;

/** Lock for concurrency */
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
/** Conditional variable for managing threads */
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

/**
 * Thread initialization function. Determines the process id to print based upon the thread-specified
 * parameter. The order of output will be dependent upon the type of scheduling policy (either
 * FIFO, RR, or OTHER)
 * @param param the thread id specifying the process id to print
 */
void *child(void *param)
{
  /** Integer specifying the id to print */
  int i = (size_t) param;
  char id;
  if (i == 0)
    id = 'A';
  else if (i == 1)
    id = 'B';
  else if (i == 2)
    id = 'C';
  else
    id = 'D';
  
  // start only after all threads are created
  pthread_mutex_lock(&lock);
  while(start == 0){
    pthread_cond_wait(&cond,&lock);
  }
  pthread_mutex_unlock(&lock);
  
  // Print the process' id
  for(i = 0; i < 200; i++){
    printf("%c", id);
    for (int t = 0; t < 1000000; t++);
  }
  return NULL;
}

/**
 * Returns the user-selected scheduling process. A user can specify 0 = FIFO,
 * 1 = RR, and 2 = SCHED_OTHER. If an invalid selection is made, SCHED_OTHER
 * is set.
 * @param input the user-specified shcheduling policy
 * @return the scheduling policy corresponding to the user's selection
 */
int selection(int input){
  if (input==0){
    return SCHED_FIFO;
  } else if(input==1){
    return  SCHED_RR;
  } else if(input==2){
    return SCHED_OTHER;
  } else{
    printf("Invalid selection: %d", input);
    return SCHED_OTHER;
  }
}

/** 
 * Executes three processes through a user-specified scheduling policy using threads.
 * As each process executes, its associated id is printed to standard output.
 * Once each thread completes, the function joins all threads and terminates.
 */
int main(int argc, char *argv[])
{
  int i, policy;
  if(argc != 2){
    printf("Indicate the scheduling policy type");
    exit(1);
  }
  policy = atoi(argv[1]);

  /** Array of threads to manage processes */
  pthread_t tid[NUM_THREADS];
  // Scheduling attributes
  pthread_attr_t attr;
  pthread_attr_t attr2;
  // Initialize each scheduling attribute
  pthread_attr_init(&attr);
  pthread_attr_init(&attr2);
  pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
  pthread_attr_setinheritsched(&attr2, PTHREAD_EXPLICIT_SCHED);
  // Interprete argument - argv[1] 
  policy = selection(policy);

  /********************************************/
  /* Set scheduling policy in attr            */
  /********************************************/ 
  pthread_attr_setschedpolicy(&attr, policy);
  pthread_attr_setschedpolicy(&attr2, policy);
  /********************************************/
  /* Set scheduling priority in attr          */
  /********************************************/ 
  
  /** Struct representing scheduling parameters */
  struct sched_param param;
  // Set the scheduling parameters for both the high and low priority threads
  param.sched_priority = sched_get_priority_max(policy);
  pthread_attr_setschedparam(&attr, &param);
  param.sched_priority = sched_get_priority_min(policy);
  pthread_attr_setschedparam(&attr2, &param);

  // create the threads using the specified attr
    if (pthread_create(&tid[0], &attr, child, (void *)0) != 0){
        fprintf(stderr, "Unable to create thread.\n");
    }
    if (pthread_create(&tid[1], &attr2, child, (void *)1) != 0){
        fprintf(stderr, "Unable to create thread.\n");
    }
    if (pthread_create(&tid[2], &attr2, child, (void *)2) != 0){
        fprintf(stderr, "Unable to create thread.\n");
    }
  // code to set affinity for all threads so that they all run on the same processor core 
  cpu_set_t  mask;
  CPU_ZERO(&mask);
  CPU_SET(0, &mask);
  if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &mask) != 0)
    fprintf(stderr, "Unable to set affinity.\n");
  if (pthread_setaffinity_np(tid[0], sizeof(cpu_set_t), &mask) != 0)
    fprintf(stderr, "Unable to set affinity.\n");
  if (pthread_setaffinity_np(tid[1], sizeof(cpu_set_t), &mask) != 0)
    fprintf(stderr, "Unable to set affinity.\n");
  if (pthread_setaffinity_np(tid[2], sizeof(cpu_set_t), &mask) != 0)
    fprintf(stderr, "Unable to set affinity.\n");

  pthread_mutex_lock(&lock);
  start = 1;
  pthread_mutex_unlock(&lock);
  pthread_cond_broadcast(&cond);

  // now join on each thread
  for (i = 0; i < NUM_THREADS; i++)
    pthread_join(tid[i], NULL);
  // destroy each attribute and check for error
  if (pthread_attr_destroy(&attr) != 0)
    fprintf(stderr, "Unable to destroy attr.\n");
  if (pthread_attr_destroy(&attr2) != 0)
    fprintf(stderr, "Unable to destroy attr.\n");
  // print successful completion
  printf("\nP3 completes!\n");
  
  return EXIT_SUCCESS;
}
