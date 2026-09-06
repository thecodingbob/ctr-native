#include <common.h>

void LIST_Clear(struct LinkedList *L)
{
	L->first = 0;
	L->last = 0;
	L->count = 0;
}

void LIST_AddFront(struct LinkedList *L, struct Item *I)
{
	if (I == 0)
	{
		return;
	}

	I->prev = 0;

	struct Item *oldFirst = L->first;
	I->next = oldFirst;

	if (oldFirst != 0)
	{
		L->first->prev = I;
	}
	else
	{
		L->last = I;
	}

	L->first = I;
	L->count = L->count + 1;
}

void LIST_AddBack(struct LinkedList *L, struct Item *I)
{
	if (I == 0)
	{
		return;
	}

	I->next = 0;

	struct Item *oldLast = L->last;
	I->prev = oldLast;

	if (oldLast != 0)
	{
		L->last->next = I;
	}
	else
	{
		L->first = I;
	}

	L->last = I;
	L->count = L->count + 1;
}

void *LIST_GetNextItem(struct Item *I)
{
	return I->next;
}

void *LIST_GetFirstItem(struct LinkedList *L)
{
	return L->first;
}

struct Item *LIST_RemoveMember(struct LinkedList *L, struct Item *I)
{
	if (I == 0)
	{
		return 0;
	}

	if (L->first != 0)
	{
		if (I->prev != 0)
		{
			I->prev->next = I->next;
		}
		else
		{
			L->first = I->next;
		}

		if (I->next != 0)
		{
			I->next->prev = I->prev;
		}
		else
		{
			L->last = I->prev;
		}

		L->count = L->count - 1;
	}

	I->next = 0;
	I->prev = 0;

	return I;
}

struct Item *LIST_RemoveFront(struct LinkedList *L)
{
	struct Item *I = L->first;

	if (I == 0)
	{
		return 0;
	}

	if (I->prev != 0)
	{
		I->prev->next = I->next;
	}
	else
	{
		L->first = I->next;
	}

	if (I->next != 0)
	{
		I->next->prev = I->prev;
	}
	else
	{
		L->last = I->prev;
	}

	L->count = L->count - 1;
	I->next = 0;
	I->prev = 0;

	return I;
}

struct Item *LIST_RemoveBack(struct LinkedList *L)
{
	struct Item *I = L->last;

	if (I == 0)
	{
		return 0;
	}

	if (L->first != 0)
	{
		if (I->prev != 0)
		{
			I->prev->next = I->next;
		}
		else
		{
			L->first = I->next;
		}

		if (I->next != 0)
		{
			I->next->prev = I->prev;
		}
		else
		{
			L->last = I->prev;
		}

		L->count = L->count - 1;
	}

	I->next = 0;
	I->prev = 0;

	return I;
}

void LIST_Init(struct LinkedList *L, struct Item *item, int itemSize, int numItems)
{
	while (numItems > 0)
	{
		LIST_AddBack(L, item);

		numItems--;
		item = (struct Item *)((s32)item + itemSize);
	}
}
