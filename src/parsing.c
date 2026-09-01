#include "codexion.h"

static int	arg_error(char *msg)
{
	fprintf(stderr, "%s\n", msg);
	return (1);
}

static int	to_num(char *str, long *s)
{
	int		i;
	long	n;

	i = 0;
	n = 0;
	if (!str[i])
		return (1);
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (1);
		n = n * 10 + (str[i] - '0');
		if (n > INT_MAX)
			return (1);
		i++;
	}
	*s = n;
	return (0);
}

int	parse_args(t_sim *s, char **argv)
{
	if (to_num(argv[1], &s->coders_nbr) || s->coders_nbr < 1)
		return (arg_error("Coders number must be positive integer"));
	if (to_num(argv[2], &s->burnout_time) || s->burnout_time < 1)
		return (arg_error("Time to burnout must be positive integer"));
	if (to_num(argv[3], &s->compile_time))
		return (arg_error("Time to compile must be a valid integer"));
	if (to_num(argv[4], &s->debug_time))
		return (arg_error("Time to debug must be a valid integer"));
	if (to_num(argv[5], &s->refactor_time))
		return (arg_error("Time to refactor must be a valid integer"));
	if (to_num(argv[6], &s->compiles_required) || s->compiles_required < 1)
		return (arg_error("compile number must be positive integer"));
	if (to_num(argv[7], &s->dongle_cooldown))
		return (arg_error("dongle cooldown must be a valid integer"));
	s->is_edf = 0;
	if (strcmp("edf", argv[8]) == 0)
		s->is_edf = 1;
	else if (strcmp("fifo", argv[8]))
		return (arg_error("it's either edf or fifo"));
	return (0);
}
