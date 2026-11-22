#ifndef BOTS_ADVENTURE_GRID_H
#define BOTS_ADVENTURE_GRID_H

// Put the humans at the back (P1 last) and assign only the bots that can spawn.
static inline void BOTS_SeedAdventureGrid(char *order, int players, int racers, int championDriver,
                                          struct RngDeadCoedState *rng)
{
	for (int i = 0; i < players; i++)
	{
		order[i] = racers - 1 - i;
	}
	for (int i = players; i < racers; i++)
	{
		order[i] = i - players;
	}
	if (championDriver >= players && championDriver < racers)
	{
		// Keep the retail order: move the champion to pole, using the last
		// bot to fill the gap it leaves.
		for (int i = players; i < racers - 1; i++)
		{
			order[i]++;
		}
		order[racers - 1] = championDriver - players + 1;
		order[championDriver] = 0;
		return;
	}
	for (int i = racers - 1; i > players; i--)
	{
		const int other = players + ((u32)RngDeadCoed(rng) & 0xfff) % (i - players + 1);
		const char temp = order[i];
		order[i] = order[other];
		order[other] = temp;
	}
}

#endif
