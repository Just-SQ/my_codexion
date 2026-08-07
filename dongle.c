#include "codexion.h"

void release_dongle(t_coder *c, t_dongle *d)
{
	pthread_mutex_lock(&d->lock);
	d->held = 0;
	d->available_at = now_ms() + c->sim->args.dongle;
	pthread_cond_broadcast(&d->cond);
	pthread_mutex_unlock(&d->lock);
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

void taking_dongle(t_dongle *d, t_coder *c)
{
	heap_pop(&d->waiters);
	d->held = 1;
	pthread_mutex_unlock(&d->lock);
	log_state(c, "has taken a dongle");
}

void cond_timedwait(t_dongle *d)
{
	struct timespec ts;

	ts.tv_sec = d->available_at / 1000;
	ts.tv_nsec = (d->available_at % 1000) * 1000000;
	pthread_cond_timedwait(&d->cond, &d->lock, &ts);
}

int take_dongle(t_coder *c, t_dongle *d)
{
	t_req r;

	make_req(&r, c);
	pthread_mutex_lock(&d->lock);
	heap_push(&d->waiters, r, c->sim->args.edf);
	while (1)
	{
		if (stopped(c))
		{
			heap_remove_id(&d->waiters, r.coder_id);
			pthread_cond_broadcast(&d->cond);
			pthread_mutex_unlock(&d->lock);
			return (1);			
		}
		if (d->waiters.arr[0].coder_id == c->id
				&& !d->held && now_ms() >= d->available_at)
			return (taking_dongle(d, c), 0);
		if (!d->held && now_ms() < d->available_at)
			cond_timedwait(d);
		else
			pthread_cond_wait(&d->cond, &d->lock);
	}
	return (1);
}

int take_dongles(t_coder *c, t_dongle *first, t_dongle *second)
{
	if (first == second)
	{
		if (!take_dongle(c, first))
			release_dongle(c, first);
		return (1);
	}
	if (take_dongle(c, first))
		return (1);
	if (take_dongle(c, second))
		return (release_dongle(c, first), 1);
	return (0);
}