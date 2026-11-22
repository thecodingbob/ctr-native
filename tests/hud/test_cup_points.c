#include <shared/ctr_test.h>
#include <game/UI/UI_CupPoints.h>

static const int retail[] = {9, 6, 3, 1, 0, 0, 0, 0};

static void CheckField(int count, const int *expected)
{
	for (int position = 0; position < count; position++)
	{
		CTR_TEST_CHECK_EQ(UI_CupPoints_ForPosition(retail, 1, count - 1, position), expected[position],
		                  "each position receives the agreed cup points");
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
	return CtrTest_Summary();
}
