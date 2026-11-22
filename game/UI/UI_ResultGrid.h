#ifndef UI_RESULT_GRID_H
#define UI_RESULT_GRID_H

enum { UI_RESULT_GRID_CELL_WIDTH = 60 };

// Expanded fields use two balanced rows, each centered independently. Use the
// final field size, not the number of icons already revealed by the animation.
static inline int UI_ResultGrid_X(int left, int width, int racers, int rank, int *row)
{
	const int firstRow = (racers + 1) / 2;
	*row = rank >= firstRow;
	const int columns = *row ? racers - firstRow : firstRow;
	const int column = *row ? rank - firstRow : rank;
	return left + (width - columns * UI_RESULT_GRID_CELL_WIDTH) / 2 + column * UI_RESULT_GRID_CELL_WIDTH;
}

#endif
