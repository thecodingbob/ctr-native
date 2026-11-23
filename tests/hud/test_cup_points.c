#include <shared/ctr_test.h>
#include <game/UI/UI_CupPoints.h>
#include <game/UI/UI_ResultGrid.h>

static const int retail[] = {9, 6, 3, 1, 0, 0, 0, 0};

static void CheckField(int count, const int *expected)
{
	for (int position = 0; position < count; position++)
	{
		CTR_TEST_CHECK_EQ(UI_CupPoints_ForPosition(retail, 1, count - 1, position), expected[position],
		                  "each position receives the agreed cup points");
	}
}

static void CheckExpandedResultGrid(void)
{
	CTR_TEST_SECTION("expanded results arrange all racers in two centered rows");
	const int left = 20;
	const int width = 512;
	for (int racers = 9; racers <= 16; racers++)
	{
		const int first = (racers + 1) / 2;
		int previousX = 0;
		for (int rank = 0; rank < racers; rank++)
		{
			int row;
			int x = UI_ResultGrid_X(left, width, racers, rank, &row);
			CTR_TEST_CHECK_EQ(row, rank >= first, "ranks stay in order across the row boundary");
			CTR_TEST_CHECK(x >= left && x + UI_RESULT_GRID_CELL_WIDTH <= left + width, "each cell stays within the results viewport");
			if (rank == 0 || rank == first)
			{
				int cells = row ? racers - first : first;
				CTR_TEST_CHECK_EQ(x - left, left + width - (x + cells * UI_RESULT_GRID_CELL_WIDTH), "each row is centered independently");
			}
			else
			{
				CTR_TEST_CHECK_EQ(x - previousX, UI_RESULT_GRID_CELL_WIDTH, "adjacent cells do not overlap");
			}
			previousX = x;
		}
	}
}

int main(void)
{
	static const int nine[] = {9, 6, 4, 2, 1, 0, 0, 0, 0};
	static const int twelve[] = {9, 7, 5, 3, 2, 1, 0, 0, 0, 0, 0, 0};
	static const int sixteen[] = {9, 8, 6, 5, 3, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0};

	CTR_TEST_SECTION("retail and expanded cup scores");
	for (int count = 2; count <= 8; count++)
	{
		CheckField(count, retail);
	}
	CheckField(9, nine);
	CheckField(12, twelve);
	CheckField(16, sixteen);

	CTR_TEST_SECTION("invalid positions");
	CTR_TEST_CHECK(UI_CupPoints_ForPosition(retail, 4, 0, 0) == 3, "a player-only cup awards retail points");
	CTR_TEST_CHECK_EQ(UI_CupPoints_ForPosition(retail, 4, 0, 3), 0, "the last player in a four-player cup receives zero");
	CTR_TEST_CHECK_EQ(UI_CupPoints_ForPosition(retail, 0, 0, 0), 0, "an empty field awards no points");
	CTR_TEST_CHECK_EQ(UI_CupPoints_ForPosition(retail, 1, 15, -1), 0, "a negative position awards no points");
	CTR_TEST_CHECK_EQ(UI_CupPoints_ForPosition(retail, 1, 15, 16), 0, "a position beyond the field awards no points");
	CheckExpandedResultGrid();
	CTR_TEST_SECTION("solo result rows retain their intended alignment");
	CTR_TEST_CHECK_EQ(UI_ResultGrid_SoloY(0, 0), 96, "the retail single row does not move");
	const int iconHeight = 32 * UI_RESULT_GRID_SOLO_ICON_SCALE / 0x1000;
	CTR_TEST_CHECK_EQ(UI_ResultGrid_SoloY(1, 0), 93, "the expanded top row keeps its aligned position");
	CTR_TEST_CHECK(UI_ResultGrid_SoloY(1, 0) + iconHeight < UI_ResultGrid_SoloY(1, 1), "the two compact rows have vertical clearance");
	CTR_TEST_CHECK_EQ(UI_ResultGrid_SoloY(1, 1), 121, "the expanded bottom row keeps its aligned position");
	return CtrTest_Summary();
}
