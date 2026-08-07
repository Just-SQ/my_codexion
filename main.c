#include "codexion.h"
#include <stdio.h>

void usage()
{
	fprintf(stderr, "Error: invalid arguments\n");
	fprintf(stderr, "Usage: ./codexion number_of_coders time_to_burnout "
		"time_to_compile time_to_debug time_to_refactor "
		"number_of_compiles_required dongle_cooldown <fifo|edf>\n");
}

int	main(int argc, char **argv)
{
  t_args arg;
  t_sim s;

	if (argc != 9)
		return (usage(), 1);
	if (parse_args(&arg, argv))
		return (1);
	if (init_sim(&s, arg))
    return (fprintf(stderr, "Error in initialization\n"), 1);
	
	if (start_thread(&s))
	{
		destroy_sim(&s);
		return (fprintf(stderr, "Error: thread creation failed\n"), 1);
	}
	join_threads(&s);
	destroy_sim(&s);
}
