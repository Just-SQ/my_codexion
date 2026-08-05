#ifndef CODEXION_H
#define CODEXION_H
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct s_sim  t_sim;

typedef struct s_args
{
  long coders_nbr;
  long burnout;
  long compile;
  long debug;
  long refactor;
  long compile_nbr;
  long dongle;
  int edf;
}     t_args;

typedef struct s_req
{
  int coder_id;
  long order; // arrival order: fifo key and edf tie breaker
  long deadline; // last_compile_start + burnout: edf key
}     t_req;

typedef struct s_heap
{
  t_req arr[2];
  int size;
}     t_heap;

typedef struct s_coder
{
  int id;
  pthread_t thread;
  long last_compile_start; // ms since simulation start
  long compiles; // counter of completed compiles
  pthread_mutex_t state;
  t_sim *sim;
}     t_coder;

typedef struct s_dongle
{
  pthread_mutex_t lock;
  pthread_cond_t cond;
  int held; //1 = taken by a code, 0 = on the table
  long available_at; // release time + cooldown
  t_heap waiters;
}     t_dongle;

typedef struct s_sim
{
  t_args args;
  long start_ms;
  t_coder *coders;
  t_dongle *dongles;
  pthread_t monitor;
  int stop; // 0 while sim running, 1 stopped on burnout or coder reached compile nbr
  pthread_mutex_t state_lock;
  pthread_mutex_t log_lock;
  long ticket; // dongle request counter
}     t_sim;


int parse_args(t_args *s, char **argv);
int init_sim(t_sim *s, t_args args);

#endif