#include "codexion.h"
#include <stdio.h>

int	main(int argc, char **argv)
{
  t_args s;

	if (parse_args(&s, argv) || argc != 9)
    return (1);
  printf("number of coders: %ld\n", s.coders_nbr);
  printf("time to burnout: %ld\n", s.burnout);
  printf("time to compile: %ld\n", s.compile);
  printf("time to debug: %ld\n", s.debug);
  printf("time to refactor: %ld\n", s.refactor);
  printf("number of compiles: %ld\n", s.compile_nbr);
  printf("dongle cooldown: %ld\n", s.dongle);
  if (s.edf)
    printf("it's edf");
  else
    printf("it's fifo");
}
