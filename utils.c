#include "codexion.h"

long now_ms()
{
	struct timeval tv;

	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

int stopped(t_coder *c)
{
	int stop;

	pthread_mutex_lock(&c->sim->state_lock);
	stop = c->sim->stop;
	pthread_mutex_unlock(&c->sim->state_lock);
	return (stop);
}

void	log_state(t_coder *c, char *msg)
{
	pthread_mutex_lock(&c->sim->log_lock);
	if (stopped(c))
	{
		pthread_mutex_unlock(&c->sim->log_lock);
		return ;
	}
	printf("%ld %d %s\n", now_ms() - c->sim->start_ms, c->id, msg);
	pthread_mutex_unlock(&c->sim->log_lock);
}