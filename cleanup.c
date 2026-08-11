#include "codexion.h"

void destroy_sim(t_sim *s)
{
	int i;
	
	i = 0;
	while (i < s->args.coders_nbr)
		pthread_mutex_destroy(&s->coders[i++].state);

	pthread_mutex_destroy(&s->table);
	pthread_cond_destroy(&s->table_cond);	
	pthread_mutex_destroy(&s->state_lock);
	pthread_mutex_destroy(&s->log_lock);
	free(s->coders);
	free(s->dongles);
}

void coder_destroy(t_sim *s, int count)
{
	int i;

	i = 0;
	while (i < count)
		pthread_mutex_destroy(&s->coders[i++].state);
}
