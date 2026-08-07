#include "codexion.h"

int init_coders(t_sim *s)
{
  int i;

  i = 0;
  while (i < s->args.coders_nbr)
  {
    s->coders[i].id = i + 1;
    s->coders[i].compiles = 0;
    s->coders[i].sim = s;
    if (pthread_mutex_init(&s->coders[i].state, NULL))
      return (coder_destroy(s, i), 1);
    i++;
  }
  return (0);
}

int init_dongle(t_dongle *dongle)
{
  dongle->available_at = 0;
  dongle->held = 0;
  dongle->waiters.size = 0;
  if (pthread_mutex_init(&dongle->lock, NULL))
    return (1);
  if (pthread_cond_init(&dongle->cond, NULL))
  {
    pthread_mutex_destroy(&dongle->lock);
    return (1);
  }
  return (0);
}

int init_dongles(t_sim *s)
{
  int i;

  i = 0;
  while (i < s->args.coders_nbr)
  {
    if (init_dongle(&s->dongles[i]))
      return (dongle_destroy(s, i), coder_destroy(s, s->args.coders_nbr), 1);
    i++;
  }
  return (0);
}

int init_sim(t_sim *s, t_args args)
{
  s->args = args;
  s->stop = 0;
  s->ticket = 0;
  s->coders = malloc(sizeof(t_coder) * args.coders_nbr);
  s->dongles = malloc(sizeof(t_dongle) * args.coders_nbr);
  if (!s->coders || !s->dongles)
    return(free(s->coders), free(s->dongles), 1);
  if (pthread_mutex_init(&s->state_lock, NULL))
    return(free(s->coders), free(s->dongles), 1);
  if (pthread_mutex_init(&s->log_lock, NULL))
  {
    pthread_mutex_destroy(&s->state_lock);
    return(free(s->coders), free(s->dongles), 1);
  }
  if (init_coders(s) || init_dongles(s))
  {
    pthread_mutex_destroy(&s->state_lock);
    pthread_mutex_destroy(&s->log_lock);
    return (free(s->coders),free(s->dongles), 1);
  }
  return (0);
}