#include "codexion.h"

static void	cancel_start(t_sim *s, int count)
{
	int	i;

	i = 0;
	pthread_mutex_lock(&s->data);
	s->is_stopped = 1;
	pthread_mutex_unlock(&s->data);

	pthread_mutex_lock(&s->table);
	pthread_cond_broadcast(&s->cond);
	pthread_mutex_unlock(&s->table);

	while (i < count)
	{
		pthread_join(s->coders[i].thread, NULL);
		i++;
	}
}

long	now_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

int	start_threads(t_sim *s)
{
	int	i;

	i = 0;
	s->start_ms = now_ms();
	while (i < s->coders_nbr)
	{
		s->coders[i].last_start = s->start_ms;
		if (pthread_create(&s->coders[i].thread, NULL,
				coder_routine, &s->coders[i]))
			return (cancel_start(s, i), 1);
		i++;
	}
	if (pthread_create(&s->monitor, NULL, monitor_routine, s))
		return (cancel_start(s, i), 1);
	return (0);
}

void	join_threads(t_sim *s)
{
	int	i;

	i = 0;
	while (i < s->coders_nbr)
		pthread_join(s->coders[i++].thread, NULL);
	pthread_join(s->monitor, NULL);
}