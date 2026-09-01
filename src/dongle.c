#include "codexion.h"

static void	push_req(t_coder *c, t_dongle *first, t_dongle *second)
{
	t_request	req;

	req.id = c->id;
	req.order = c->sim->ticket++;
	pthread_mutex_lock(&c->sim->data);
	req.deadline = c->last_start + c->sim->burnout_time;
	pthread_mutex_unlock(&c->sim->data);
	heap_push(&first->waiters, req, c->sim->is_edf);
	heap_push(&second->waiters, req, c->sim->is_edf);
}

static void	wait_table(t_sim *s, t_dongle *first, t_dongle *second)
{
	long			wake;
	struct timespec	ts;

	if (first->free_at > second->free_at)
		wake = first->free_at;
	else
		wake = second->free_at;
	if (wake <= now_ms())
		pthread_cond_wait(&s->cond, &s->table);
	else
	{
		ts.tv_sec = wake / 1000;
		ts.tv_nsec = (wake % 1000) * 1000000;
		pthread_cond_timedwait(&s->cond, &s->table, &ts);
	}
}

int	take_dongles(t_coder *c, t_dongle *first, t_dongle *second)
{
	pthread_mutex_lock(&c->sim->table);
	push_req(c, first, second);
	while (!stopped(c->sim))
	{
		if (!can_take(first, c->sim, c->id) || !can_take(second, c->sim, c->id))
		{
			wait_table(c->sim, first, second);
			continue;
		}
		remove_id(&first->waiters, c->id);
		remove_id(&second->waiters, c->id);
		first->is_taken = 1;
		second->is_taken = 1;
		pthread_mutex_lock(&c->sim->data);
		c->last_start = now_ms();
		pthread_mutex_unlock(&c->sim->data);
		pthread_mutex_unlock(&c->sim->table);
		return (0);
	}
	pthread_mutex_unlock(&c->sim->table);
	return (1);
}

void release_dongles(t_coder *c, t_dongle *first, t_dongle *second)
{
	long	now;

	now = now_ms();
	pthread_mutex_lock(&c->sim->table);
	first->is_taken = 0;
	first->free_at = now + c->sim->dongle_cooldown;
	second->is_taken = 0;
	second->free_at = now + c->sim->dongle_cooldown;
	pthread_cond_broadcast(&c->sim->cond);
	pthread_mutex_unlock(&c->sim->table);
}