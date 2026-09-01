#include "codexion.h"

static int	compile(t_coder *c, t_dongle *first, t_dongle *second)
{
	if (!take_dongles(c, first, second))
	{
		state_log("has taken a dongle", c->id, c->sim);

		usleep(c->sim->compile_time * 1000);
		pthread_mutex_lock(&c->sim->data);
		c->compiles++;
		pthread_mutex_unlock(&c->sim->data);
		release_dongles(c, first, second);
		return (0);
	}
	return (1);
}

static void	one_coder(t_coder *c)
{
	pthread_mutex_lock(&c->sim->log);
	printf("%ld %d has taken a dongle\n", now_ms() - c->sim->start_ms, c->id);
	pthread_mutex_unlock(&c->sim->log);
	while (!stopped(c->sim))
		usleep(300);
}

int	stopped(t_sim *s)
{
	int	stop;

	pthread_mutex_lock(&s->data);
	stop = s->is_stopped;
	pthread_mutex_unlock(&s->data);
	return (stop);
}

void	state_log(char *str, int id, t_sim *s)
{
	pthread_mutex_lock(&s->log);
	if (!stopped(s))
	{
		if (!strcmp(str, "has taken a dongle"))
		{
			printf("%ld %d has taken a dongle\n", now_ms() - s->start_ms, id);
			printf("%ld %d has taken a dongle\n", now_ms() - s->start_ms, id);
			printf("%ld %d is compiling\n", now_ms() - s->start_ms, id);
		}
		else
			printf("%ld %d %s\n", now_ms() - s->start_ms, id, str);
	}
	pthread_mutex_unlock(&s->log);
}

void	*coder_routine(void *arg)
{
	t_coder		*c;
	t_dongle	*first;
	t_dongle	*second;

	c = arg;
	first = &c->sim->dongles[c->id - 1];
	second = &c->sim->dongles[c->id % c->sim->coders_nbr];
	if (c->sim->coders_nbr == 1)
		return (one_coder(c), NULL);
	while (!stopped(c->sim))
	{
		if (compile(c, first, second))
			return (NULL);
		state_log("is debugging", c->id, c->sim);
		usleep(c->sim->debug_time * 1000);
		state_log("is refactoring", c->id, c->sim);
		usleep(c->sim->refactor_time * 1000);
	}
	return (NULL);
}