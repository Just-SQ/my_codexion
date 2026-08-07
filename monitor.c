#include "codexion.h"

int edit_state(t_sim *s, int dead, int done)
{
	if (dead || done == s->args.coders_nbr)
	{
		pthread_mutex_lock(&s->state_lock);
		s->stop = 1;
		pthread_mutex_unlock(&s->state_lock);
		if (dead)
			log_state(&s->coders[dead - 1], "burned out");
		return (1);
	}
	return (0);
}

int check_state(t_sim *s)
{
	long now;
	int dead;
	int done;
	int i;

	now = now_ms();
	dead = 0;
	i = 0;
	done = 0;
	while (i < s->args.coders_nbr)
	{
		pthread_mutex_lock(&s->coders[i].state);
		if (s->coders[i].compiles >= s->args.compile_nbr)
			done++;
		if (now - s->coders[i].last_compile_start >= s->args.burnout)
		{
			dead = i + 1;
			pthread_mutex_unlock(&s->coders[i].state);
			break;
		}
		pthread_mutex_unlock(&s->coders[i].state);
		i++;
	}
	return (edit_state(s, dead, done));
}


void *monitor_routine(void *arg)
{
	t_sim *s;

	s = arg;
	while(!check_state(s))
		usleep(500);
	return (NULL);
}