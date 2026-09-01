#include "codexion.h"

static void	heap_swap(t_heap *h)
{
	t_request	tmp;

	tmp = h->arr[0];
	h->arr[0] = h->arr[1];
	h->arr[1] = tmp;
}

static int	is_swap(t_heap *h, t_request req, int is_edf)
{
	if (is_edf && h->arr[0].deadline != req.deadline)
		return (h->arr[0].deadline > req.deadline);
	return (h->arr[0].order > req.order);
}

static void	heap_pop(t_heap *h)
{
	if (h->size == 1)
		h->size--;
	else if (h->size == 2)
	{
		h->arr[0] = h->arr[1];
		h->size--;
	}
}

void	heap_push(t_heap *h, t_request req, int is_edf)
{
	if (h->size >= 2)
		return;
	if (!h->size)
	{
		h->arr[0] = req;
		h->size++;
		return;
	}
	h->arr[1] = req;
	h->size++;
	if (is_swap(h, req, is_edf))
		heap_swap(h);
}

void	remove_id(t_heap *h, int id)
{
	if (h->size && h->arr[0].id == id)
		heap_pop(h);
	else if (h->size == 2 && h->arr[1].id == id)
		h->size--;
}
