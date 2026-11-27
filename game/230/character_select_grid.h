#ifndef CTR_CHARACTER_SELECT_GRID_H
#define CTR_CHARACTER_SELECT_GRID_H

#include "character_grid_types.h"
#include <stdbool.h>

enum
{
	CHARACTER_SELECT_TITLE_TRANSITION = CHARACTER_SELECT_RETAIL_SLOT_COUNT + CHARACTER_SELECT_EXTRA_SLOT_CAPACITY,
	CHARACTER_SELECT_FIRST_WINDOW_TRANSITION = CHARACTER_SELECT_TITLE_TRANSITION + 1,
};

static inline int CharacterSelectGrid_At(const struct CharacterGrid *grid, struct CharacterGridPosition position)
{
	if (position.row < 0 || position.row >= CHARACTER_GRID_ROWS ||
	    position.column < 0 || position.column >= CHARACTER_GRID_COLUMNS)
	{
		return CHARACTER_GRID_UNUSABLE;
	}
	return grid->cells[position.row][position.column];
}

static inline struct CharacterGridPosition CharacterSelectGrid_Find(const struct CharacterGrid *grid, int entry)
{
	for (int row = 0; row < CHARACTER_GRID_ROWS; row++)
	{
		for (int column = 0; column < CHARACTER_GRID_COLUMNS; column++)
		{
			if (grid->cells[row][column] == entry)
			{
				return (struct CharacterGridPosition){row, column};
			}
		}
	}
	return (struct CharacterGridPosition){-1, -1};
}

static inline void CharacterSelectGrid_FillCustom(struct CharacterGrid *grid, int count)
{
	if (count > CHARACTER_SELECT_EXTRA_SLOT_CAPACITY) { count = CHARACTER_SELECT_EXTRA_SLOT_CAPACITY; }
	for (int row = 0; row < CHARACTER_GRID_ROWS; row++)
	{
		for (int column = 0; column < CHARACTER_GRID_COLUMNS; column++)
		{
			if (grid->cells[row][column] >= CHARACTER_SELECT_RETAIL_SLOT_COUNT)
			{
				grid->cells[row][column] = CHARACTER_GRID_EMPTY;
			}
		}
	}
	int entry = CHARACTER_SELECT_RETAIL_SLOT_COUNT;
	if (count > 0 && CharacterSelectGrid_At(grid, grid->firstCustom) == CHARACTER_GRID_EMPTY)
	{
		grid->cells[grid->firstCustom.row][grid->firstCustom.column] = entry++;
		count--;
	}
	// Remaining entries use the free cells without moving any retail character.
	for (int row = 0; row < CHARACTER_GRID_ROWS; row++)
	{
		for (int column = CHARACTER_GRID_COLUMNS - 1; column >= 0 && count > 0; column--)
		{
			if (grid->cells[row][column] == CHARACTER_GRID_EMPTY)
			{
				grid->cells[row][column] = entry++;
				count--;
			}
		}
	}
}

static inline int CharacterSelectGrid_Next(const struct CharacterGrid *grid, int entry, int direction, const bool *selectable)
{
	struct CharacterGridPosition position = CharacterSelectGrid_Find(grid, entry);
	if (position.row < 0 || direction < 0 || direction >= CHARACTER_SELECT_DIRECTION_COUNT) { return entry; }
	int rowStep = direction == CHARACTER_SELECT_DIR_UP ? -1 : direction == CHARACTER_SELECT_DIR_DOWN ? 1 : 0;
	int columnStep = direction == CHARACTER_SELECT_DIR_LEFT ? -1 : direction == CHARACTER_SELECT_DIR_RIGHT ? 1 : 0;
	position.row += rowStep;
	position.column += columnStep;
	while (position.row >= 0 && position.row < CHARACTER_GRID_ROWS && position.column >= 0 && position.column < CHARACTER_GRID_COLUMNS)
	{
		int candidate = CharacterSelectGrid_At(grid, position);
		if (candidate >= 0 && candidate < CHARACTER_SELECT_TITLE_TRANSITION && selectable[candidate]) { return candidate; }
		position.row += rowStep;
		position.column += columnStep;
	}
	return entry;
}

#endif
