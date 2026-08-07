#include "codexion.h"

void cancel_start(t_sim *s, int count)
{
	int i;

	i = 0;

	pthread_mutex_lock(&s->state_lock);
	s->stop = 1;
	pthread_mutex_unlock(&s->state_lock);

	while (i < count)
		pthread_join(s->coders[i++].thread, NULL);
}

int start_thread(t_sim *s)
{
	int i;
	
	i = 0;
	s->start_ms = now_ms();
	while (i < s->args.coders_nbr)
		s->coders[i++].last_compile_start = s->start_ms;
	
	i = 0;
	while (i < s->args.coders_nbr)
	{
		if (pthread_create(&s->coders[i].thread, NULL, coder_routine, &s->coders[i]))
			return (cancel_start(s, i), 1);
		i++;
	}
	
	if (pthread_create(&s->monitor, NULL, monitor_routine, s))
		return (cancel_start(s, i), 1);
	
	return (0);
}

void join_threads(t_sim *s)
{
	int i;

	i = 0;
	while (i < s->args.coders_nbr)
		pthread_join(s->coders[i++].thread, NULL);
	pthread_join(s->monitor, NULL);
}