#include "codexion.h"
#include <stdio.h>

int	main(int argc, char **argv)
{
  t_args arg;
  t_sim s;

	if (argc != 9 || parse_args(&arg, argv))
    return (fprintf(stderr, "Error: invalid arguments\n"), 1);
  if (init_sim(&s, arg))
    return (fprintf(stderr, "Error in initialization\n"), 1);
  // printf("number of coders: %ld\n", s.coders_nbr);
  // printf("time to burnout: %ld\n", s.burnout);
  // printf("time to compile: %ld\n", s.compile);
  // printf("time to debug: %ld\n", s.debug);
  // printf("time to refactor: %ld\n", s.refactor);
  // printf("number of compiles: %ld\n", s.compile_nbr);
  // printf("dongle cooldown: %ld\n", s.dongle);
  // if (s.edf)
  //   printf("it's edf");
  // else
  //   printf("it's fifo");
}
