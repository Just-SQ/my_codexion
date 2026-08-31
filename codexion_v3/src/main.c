#include "codexion.h"

void usage()
{
	fprintf(stderr, "Error: invalid arguments\n");
	fprintf(stderr, "Usage: ./codexion number_of_coders time_to_burnout "
		"time_to_compile time_to_debug time_to_refactor "
		"number_of_compiles_required dongle_cooldown <fifo|edf>\n");
}

void	cleanup(t_sim *s)
{
	pthread_mutex_destroy(&s->table);
	pthread_mutex_destroy(&s->log);
	pthread_mutex_destroy(&s->data);
	pthread_cond_destroy(&s->cond);
	free(s->coders);
	free(s->dongles);
}

int	main(int argc, char **argv)
{
  t_sim s;

	memset(&s, 0, sizeof(t_sim));
	if (argc != 9)
		return (usage(), 1);
	if (parse_args(&s, argv))
		return (1);
	if (init_sim(&s))
		return (1);
	if (start_threads(&s))
	{
		fprintf(stderr, "failed to create a thread\n");
		return (1);
	}
	join_threads(&s);

	return (cleanup(&s), 0);
}
