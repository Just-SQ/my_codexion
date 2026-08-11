#include "codexion.h"

void release_dongles(t_coder *c, t_dongle *first, t_dongle *second)
{
	pthread_mutex_lock(&c->sim->table);
	first->held = 0;
	second->held = 0;
	first->available_at = now_ms() + c->sim->args.dongle;
	second->available_at = now_ms() + c->sim->args.dongle;
	pthread_cond_broadcast(&c->sim->table_cond);
	pthread_mutex_unlock(&c->sim->table);
}

void make_req(t_req *r, t_coder *c)
{
	r->coder_id = c->id;
	pthread_mutex_lock(&c->state);
	r->deadline = c->last_compile_start + c->sim->args.burnout;
	pthread_mutex_unlock(&c->state);
	pthread_mutex_lock(&c->sim->state_lock);
	r->order = c->sim->ticket++;
	pthread_mutex_unlock(&c->sim->state_lock);
}

void taking_dongle(t_coder *c, t_dongle *first, t_dongle *second)
{
	heap_remove_id(&first->waiters, c->id);
	heap_remove_id(&second->waiters, c->id);
	first->held = 1;
	second->held = 1;
	pthread_mutex_unlock(&c->sim->table);
	pthread_mutex_lock(&c->sim->log_lock);
	if (stopped(c))
	{
		pthread_mutex_unlock(&c->sim->log_lock);
		return ;
	}
	printf("%ld %d has taken a dongle\n", now_ms() - c->sim->start_ms, c->id);
	printf("%ld %d has taken a dongle\n", now_ms() - c->sim->start_ms, c->id);
	printf("%ld %d is compiling\n", now_ms() - c->sim->start_ms, c->id);
	pthread_mutex_unlock(&c->sim->log_lock);
}

void cond_timedwait(t_coder *c, long available_at)
{
	struct timespec ts;

	ts.tv_sec = available_at / 1000;
	ts.tv_nsec = (available_at % 1000) * 1000000;
	pthread_cond_timedwait(&c->sim->table_cond, &c->sim->table, &ts);
}

int	pair_free(t_sim *s, int id)
{
	t_dongle	*a;
	t_dongle	*b;

	a = &s->dongles[id - 1];
	b = &s->dongles[id % s->args.coders_nbr];
	return (!a->held && now_ms() >= a->available_at
		&& !b->held && now_ms() >= b->available_at);
}


int can_take(t_coder *c, t_dongle *d)
{
	// return (d->waiters.size > 0
	// 				&& d->waiters.arr[0].coder_id == c->id
	// 				&& !d->held
	// 				&& now_ms() >= d->available_at);
	int	head;

	if (d->held || now_ms() < d->available_at || d->waiters.size == 0)
		return (0);
	head = d->waiters.arr[0].coder_id;
	if (head == c->id)
		return (1);
	return (!pair_free(c->sim, head));
}

long max(long a, long b)
{
	if (a >= b)
		return (a);
	return (b);
}

int take_dongles(t_coder *c, t_dongle *first, t_dongle *second)
{
	t_req r;

	if (first == second)
	{
		log_state(c, "has taken a dongle");
		pthread_mutex_lock(&c->sim->state_lock);
		c->sim->stop = 1;
		pthread_mutex_unlock(&c->sim->state_lock);
		return (1);
	}
	make_req(&r, c);
	pthread_mutex_lock(&c->sim->table);
	heap_push(&first->waiters, r, c->sim->args.edf);
	heap_push(&second->waiters, r, c->sim->args.edf);
	while (1)
	{
		if (stopped(c))
		{
			heap_remove_id(&first->waiters, r.coder_id);
			heap_remove_id(&second->waiters, r.coder_id);
			pthread_cond_broadcast(&c->sim->table_cond);
			pthread_mutex_unlock(&c->sim->table);
			return (1);			
		}
		if (can_take(c, first) && can_take(c, second))
			return (taking_dongle(c, first, second), 0);
		if (!first->held && !second->held)
			cond_timedwait(c, max(first->available_at, second->available_at));
		else
			pthread_cond_wait(&c->sim->table_cond, &c->sim->table);
	}
	return (1);
}
