#include <common.h>

struct CheckpointNode *RB_Warpball_NewPathNode(struct CheckpointNode *cn, struct Driver *d)
{
	struct CheckpointNode *currNode;
	struct CheckpointNode *nodes;
	b16 foundLeftPath;
	s16 i;

	foundLeftPath = 0;
	if (d != NULL)
	{
		currNode = cn;
		if (d->checkpoint.branchChoiceIndex == cn->nextIndex_left)
		{
			return &GAME_TRACKER->level1->ptr_restart_points[cn->nextIndex_left];
		}
		if (cn->nextIndex_left != 0xff)
		{
			nodes = GAME_TRACKER->level1->ptr_restart_points;
			for (i = 0; i < 3; i++)
			{
				currNode = currNode->nextIndex_left == 0xff ? &nodes[currNode->nextIndex_forward] : &nodes[currNode->nextIndex_left];
				if (d->checkpoint.branchChoiceIndex == currNode->nextIndex_forward)
				{
					foundLeftPath = 1;
					break;
				}
			}
		}
		if (foundLeftPath)
		{
			return &GAME_TRACKER->level1->ptr_restart_points[cn->nextIndex_left];
		}
	}
	return &GAME_TRACKER->level1->ptr_restart_points[cn->nextIndex_forward];
}
