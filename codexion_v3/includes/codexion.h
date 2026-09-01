#ifndef CODEXION_H
#define CODEXION_H
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct s_sim	t_sim;

typedef struct s_request
{
	int	id;
	long order; // arrival order: fifo key and edf tie breaker
  long deadline; // last_compile_start + burnout: edf key
}			t_request;

typedef struct s_heap
{
	t_request	arr[2];
	int	size;
}			t_heap;

typedef struct s_coder
{
	int		id;
	int		compiles;
	long	last_start;
	pthread_t	thread;
	t_sim	*sim;
}			t_coder;

typedef struct s_dongle
{
	int		is_taken;
	long	free_at;
	t_heap	waiters;
}			t_dongle;

typedef struct s_sim
{
  long		coders_nbr;
  long	burnout_time;
  long	compile_time;
  long	debug_time;
  long	refactor_time;
  long	compiles_required;
  long	dongle_cooldown;
  int		is_edf;
  int		is_stopped;
  long	start_ms;
	long	ticket;
  t_coder	*coders;
  t_dongle	*dongles;
  pthread_mutex_t	table;
  pthread_mutex_t	log;
  pthread_mutex_t	data;
  pthread_cond_t	cond;
	pthread_t		monitor;
}			t_sim;

//parsing.c
int parse_args(t_sim *s, char **argv);

//init.c
int	init_sim(t_sim *s);

//start.c
long	now_ms();
int	start_threads(t_sim *s);
void	join_threads(t_sim *s);

//heap.c
void	heap_push(t_heap *h, t_request req, int is_edf);
void	remove_id(t_heap *h, int id);

//coder.c
void	*coder_routine(void *arg);
void	state_log(char *str, int id, t_sim *s);
int	stopped(t_sim *s);

//dongle.c
int	take_dongles(t_coder *c, t_dongle *first, t_dongle *second);
void release_dongles(t_coder *c, t_dongle *first, t_dongle *second);

//dongle2.c
int can_take(t_dongle *d, t_sim *s, int id);

//monitor.c
void	*monitor_routine(void *arg);

#endif