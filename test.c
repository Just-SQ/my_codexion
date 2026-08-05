#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void	*from_t(void *s)
{
  int i = 0;
  pthread_mutex_t m;
  // pthread_mutex_init(&m, NULL);
  // memset(&m, 0, sizeof(pthread_mutex_t));
  while (i < 100000000)
  {
    pthread_mutex_lock(&m);
    (*(int *)s)++;
    pthread_mutex_unlock(&m);
    i++;
  }
  // pthread_mutex_destroy(&m);
}

int	main()
{
	int p = 0;
  pthread_t t1;
  pthread_t t2;
  pthread_create(&t1, NULL, from_t, &p);
  pthread_create(&t2, NULL, from_t, &p);
  pthread_join(t1, NULL);
  pthread_join(t2, NULL);
  printf("this is p :%d", p);
}