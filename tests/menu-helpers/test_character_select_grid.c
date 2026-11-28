#include <shared/ctr_test.h>
#include <common.h>
#include <game/230/character_select_grid.h>

int main(void)
{
	struct CharacterGrid grid = {0};
	for (int row = 0; row < CHARACTER_GRID_ROWS; row++)
	{
		for (int column = 0; column < CHARACTER_GRID_COLUMNS; column++)
		{
			grid.cells[row][column] = CHARACTER_GRID_UNUSABLE;
		}
	}
	grid.cells[0][0] = CRASH_BANDICOOT;
	grid.cells[0][1] = CHARACTER_GRID_EMPTY;
	grid.cells[0][3] = NEO_CORTEX;
	grid.cells[0][5] = TINY_TIGER;
	grid.cells[2][0] = COCO_BANDICOOT;
	grid.cells[2][1] = N_GIN;
	for (int column = 0; column < 7; column++) { grid.cells[3][column] = CHARACTER_GRID_EMPTY; }
	bool selectable[CHARACTER_SELECT_TITLE_TRANSITION];
	for (int i = 0; i < CHARACTER_SELECT_TITLE_TRANSITION; i++) { selectable[i] = true; }

	CTR_TEST_SECTION("logical row and column navigation");
	CTR_TEST_CHECK(CharacterSelectGrid_Next(&grid, CRASH_BANDICOOT, CHARACTER_SELECT_DIR_RIGHT, selectable) == NEO_CORTEX,
	               "empty and unusable cells are skipped without compacting the row");
	selectable[NEO_CORTEX] = false;
	CTR_TEST_CHECK(CharacterSelectGrid_Next(&grid, CRASH_BANDICOOT, CHARACTER_SELECT_DIR_RIGHT, selectable) == TINY_TIGER,
	               "locked or occupied entries are skipped along the same axis");
	CTR_TEST_CHECK(CharacterSelectGrid_Next(&grid, CRASH_BANDICOOT, CHARACTER_SELECT_DIR_DOWN, selectable) == COCO_BANDICOOT,
	               "down stays in the logical column");
	CTR_TEST_CHECK(CharacterSelectGrid_Next(&grid, NEO_CORTEX, CHARACTER_SELECT_DIR_DOWN, selectable) == NEO_CORTEX,
	               "an irregular edge does not cause a diagonal fallback");
	CTR_TEST_CHECK(CharacterSelectGrid_Next(&grid, CRASH_BANDICOOT, CHARACTER_SELECT_DIR_LEFT, selectable) == CRASH_BANDICOOT,
	               "navigation stops at the edge without wrapping");
	struct CharacterGridPosition position = CharacterSelectGrid_Find(&grid, COCO_BANDICOOT);
	CTR_TEST_CHECK(position.row == 2 && position.column == 0 && CharacterSelectGrid_At(&grid, position) == COCO_BANDICOOT,
	               "cursor coordinates identify the roster entry");

	CTR_TEST_SECTION("eight custom cells are distinct from unavailable positions");
	CharacterSelectGrid_FillCustom(&grid, CHARACTER_SELECT_EXTRA_SLOT_CAPACITY);
	int extras = 0;
	for (int row = 0; row < CHARACTER_GRID_ROWS; row++)
	{
		for (int column = 0; column < CHARACTER_GRID_COLUMNS; column++)
		{
			extras += grid.cells[row][column] >= CHARACTER_SELECT_RETAIL_SLOT_COUNT;
		}
	}
	CTR_TEST_CHECK(extras == CHARACTER_SELECT_EXTRA_SLOT_CAPACITY, "all eight empty cells can hold custom roster entries");
	CTR_TEST_CHECK(grid.cells[0][2] == CHARACTER_GRID_UNUSABLE && grid.cells[0][0] == CRASH_BANDICOOT,
	               "custom filling never overwrites unavailable or retail cells");
	CharacterSelectGrid_FillCustom(&grid, 1);
	CTR_TEST_CHECK(grid.cells[0][1] == CHARACTER_SELECT_RETAIL_SLOT_COUNT && grid.cells[3][0] == CHARACTER_GRID_EMPTY,
	               "changing the active count releases unused custom cells");
	CharacterSelectGrid_FillCustom(&grid, 0);
	CTR_TEST_CHECK(grid.cells[0][1] == CHARACTER_GRID_EMPTY, "removing customs restores the empty cell");
	grid.firstCustom = (struct CharacterGridPosition){3, 2};
	CharacterSelectGrid_FillCustom(&grid, 1);
	CTR_TEST_CHECK(grid.cells[3][2] == CHARACTER_SELECT_RETAIL_SLOT_COUNT && grid.cells[0][1] == CHARACTER_GRID_EMPTY,
	               "the first custom keeps its chosen layout position instead of filling an earlier row");
	return CtrTest_Summary();
}
