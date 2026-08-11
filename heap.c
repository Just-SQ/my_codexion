#include "codexion.h"

void heap_swap(t_heap *h)
{
	t_req tmp;

	tmp = h->arr[0];
	h->arr[0] = h->arr[1];
	h->arr[1] = tmp;
}

int is_swap(t_req a, t_req b, int edf)
{
	if (edf && a.deadline != b.deadline)
		return (a.deadline > b.deadline);
	else
		return (a.order > b.order);
}

void heap_push(t_heap *h, t_req r, int edf)
{
	if (h->size >= 2)
		return ;
	if (!h->size)
	{
		h->arr[0] = r;
		h->size++;
		return ;
	}
	h->arr[h->size] = r;
	h->size++;
	if (is_swap(h->arr[0], h->arr[1], edf))
		return (heap_swap(h));
}

void heap_pop(t_heap *h)
{
	if (h->size == 1)
		h->size--;
	else if (h->size == 2)
	{
		h->arr[0] = h->arr[1];
		h->size--;
	}
}

void	heap_remove_id(t_heap *h, int id)
{
	if (h->size > 0 && h->arr[0].coder_id == id)
		heap_pop(h);
	else if (h->size == 2 && h->arr[1].coder_id == id)
		h->size--;
}