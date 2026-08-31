#include "codexion.h"

static void	init_coders(t_sim *s)
{
	int	i;

	i = 0;
	while (i < s->coders_nbr)
	{
		s->coders[i].id = i + 1;
		s->coders[i].compiles = 0;
		s->coders[i].sim = s;
		i++;
	}
}

static void	init_dongles(t_sim *s)
{
	int	i;

	i = 0;
	while (i < s->coders_nbr)
	{
		s->dongles[i].is_taken = 0;
		s->dongles[i].free_at = 0;
		s->dongles[i].waiters.size = 0;
		i++;
	}
}

static int	init_mutexes(t_sim *s)
{
	if (pthread_mutex_init(&s->table, NULL))
		return (1);
	if (pthread_mutex_init(&s->log, NULL))
	{
		pthread_mutex_destroy(&s->table);
		return (1);
	}
	if (pthread_mutex_init(&s->data, NULL))
	{
		pthread_mutex_destroy(&s->table);
		pthread_mutex_destroy(&s->log);
		return (1);
	}
	if (pthread_cond_init(&s->cond, NULL))
	{
		pthread_mutex_destroy(&s->table);
		pthread_mutex_destroy(&s->log);
		pthread_mutex_destroy(&s->data);
		return (1);
	}
	return (0);
}

int	init_sim(t_sim *s)
{
	s->coders = malloc(sizeof(t_coder) * s->coders_nbr);
	s->dongles = malloc(sizeof(t_dongle) * s->coders_nbr);
	if (!s->coders || !s->dongles)
	{
		fprintf(stderr, "failed to malloc\n");
		return (free(s->coders), free(s->dongles), 1);
	}
	init_dongles(s);
	init_coders(s);
	if (init_mutexes(s))
	{
		fprintf(stderr, "failed to initialize mutex\n");
		return (free(s->coders), free(s->dongles), 1);
	}
	return (0);
}