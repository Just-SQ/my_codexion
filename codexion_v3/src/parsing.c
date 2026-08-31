#include "codexion.h"


static int to_num(char *str, long *s)
{
	int i;
	long n;

	i = 0;
	n = 0;
  if (!str[i])
    return (1);
	while (str[i])
  {
    if (str[i] < '0' || str[i] >'9')
      return (1);
    n = n * 10 + (str[i] - '0');
    if (n > INT_MAX)
      return (1);
    i++;
  }
  *s = n;
  return (0);
}

int parse_args(t_sim *s, char **argv)
{
  if (to_num(argv[1], &s->coders_nbr) || s->coders_nbr < 1)
    return (fprintf(stderr, "Coders number must be positive integer\n"), 1);
  if (to_num(argv[2], &s->burnout_time) || s->burnout_time < 1)
    return (fprintf(stderr, "Time to burnout must be positive integer\n"), 1);
  if (to_num(argv[3], &s->compile_time))
    return (fprintf(stderr, "Time to compile must be a valid integer\n"), 1);
  if (to_num(argv[4], &s->debug_time))
    return (fprintf(stderr, "Time to debug must be a valid integer\n"), 1);
  if (to_num(argv[5], &s->refactor_time))
    return (fprintf(stderr, "Time to refactor must be a valid integer\n"), 1);
  if (to_num(argv[6], &s->compiles_required) || s->compiles_required < 1)
    return (fprintf(stderr, "compile number must be positive integer\n"), 1);
  if (to_num(argv[7], &s->dongle_cooldown))
    return (fprintf(stderr, "dongle cooldown must be a valid integer\n"), 1);
  s->is_edf = 0;
  if (strcmp("edf", argv[8]) == 0)
    s->is_edf = 1;
  else if (strcmp("fifo", argv[8]))
    return (fprintf(stderr, "it's either edf or fifo\n"), 1);
  return (0);
}