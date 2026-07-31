#ifndef CODEXION_H
#define CODEXION_H
#include <limits.h>
#include <stdio.h>
#include <string.h>


typedef struct s_args
{
  long coders_nbr;
  long burnout;
  long compile;
  long debug;
  long refactor;
  long compile_nbr;
  long dongle;
  int edf;
}     t_args;

int parse_args(t_args *s, char **argv);

#endif