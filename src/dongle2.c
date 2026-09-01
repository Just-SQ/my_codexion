#include "codexion.h"

static int	is_free(t_dongle *d)
{
	return (!d->is_taken && now_ms() >= d->free_at);
}

static int	can_start(t_sim *s, int id)
{
	return (is_free(&s->dongles[id - 1])
		&& is_free(&s->dongles[id % s->coders_nbr]));
}

int	can_take(t_dongle *d, t_sim *s, int id)
{
	int	head;

	if (!is_free(d))
		return (0);
	head = d->waiters.arr[0].id;
	if (head == id)
		return (1);
	return (!can_start(s, head));
}
