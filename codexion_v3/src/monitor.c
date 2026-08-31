#include "codexion.h"

static int	all_done(t_sim *s)
{
	int	i;

	i = 0;
	while (i < s->coders_nbr)
	{
		if (s->coders[i].compiles < s->compiles_required)
			return (0);
		i++;
	}
	return (1);
}

static int	find_burnout(t_sim *s, long now)
{
	int	i;

	i = 0;
	while (i < s->coders_nbr)
	{
		if (now - s->coders[i].last_start > s->burnout_time)
			return (i + 1);
		i++;
	}
	return (0);
}

static void	stop_sim(t_sim *s, int dead_id)
{
	pthread_mutex_lock(&s->log);
	pthread_mutex_lock(&s->data);
	s->is_stopped = 1;
	pthread_mutex_unlock(&s->data);
	if (dead_id)
		printf("%ld %d burned out\n", now_ms() - s->start_ms, dead_id);
	pthread_mutex_unlock(&s->log);
	pthread_mutex_lock(&s->table);
	pthread_cond_broadcast(&s->cond);
	pthread_mutex_unlock(&s->table);
}

void	*monitor_routine(void *arg)
{
	t_sim *s;
	int	dead_id;
	int	done;
	long	now;

	s = arg;
	dead_id = 0;
	while (1)
	{
		now = now_ms();
		pthread_mutex_lock(&s->data);
		done = all_done(s);
		if (!done)
			dead_id = find_burnout(s, now);	
		pthread_mutex_unlock(&s->data);
		if (done || dead_id)
			return (stop_sim(s, dead_id), NULL);
		usleep(300);
	}
}