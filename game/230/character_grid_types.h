#ifndef CTR_CHARACTER_GRID_TYPES_H
#define CTR_CHARACTER_GRID_TYPES_H

enum CharacterGridCell
{
	CHARACTER_GRID_UNUSABLE = -2,
	CHARACTER_GRID_EMPTY = -1,
};

enum
{
	CHARACTER_GRID_ROWS = 6,
	CHARACTER_GRID_COLUMNS = 8,
};

struct CharacterGridPosition
{
	short row;
	short column;
};

struct CharacterGrid
{
	short cells[CHARACTER_GRID_ROWS][CHARACTER_GRID_COLUMNS];
	short columnX[CHARACTER_GRID_COLUMNS];
	short rowY[CHARACTER_GRID_ROWS];
	struct CharacterGridPosition firstCustom;
};

#endif
