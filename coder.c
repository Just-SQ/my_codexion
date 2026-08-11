#include "codexion.h"

int compile(t_coder *c, t_dongle *first, t_dongle *second)
{
	if (!take_dongles(c, first, second))
	{
		pthread_mutex_lock(&c->state);
		c->last_compile_start = now_ms();
		pthread_mutex_unlock(&c->state);

		usleep(c->sim->args.compile * 1000);

		pthread_mutex_lock(&c->state);
		c->compiles++;
		pthread_mutex_unlock(&c->state);

		release_dongles(c, first, second);
		return (0);
	}
	return (1);
}

void *coder_routine(void *arg)
{
	t_coder *c;
	t_dongle *first;
	t_dongle *second;
	t_dongle *tmp;

	c = arg;
	first = &c->sim->dongles[c->id - 1];
	second = &c->sim->dongles[c->id % c->sim->args.coders_nbr];
	if (second < first)
	{
		tmp = first;
		first = second;
		second = tmp;
	}
	while (!stopped(c))
	{
		if (compile(c, first, second))
			return (NULL);
		log_state(c, "is debugging");
		usleep(c->sim->args.debug * 1000);
		log_state(c, "is refactoring");
		usleep(c->sim->args.refactor * 1000);
	}
	return (NULL);
}
