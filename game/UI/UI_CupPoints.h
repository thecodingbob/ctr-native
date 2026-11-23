#ifndef UI_CUP_POINTS_H
#define UI_CUP_POINTS_H

enum
{
	UI_CUP_POINTS_RETAIL_RACERS = 8,
	UI_CUP_POINTS_RETAIL_AWARD_POSITIONS = 4,
};

// Sample the retail points curve across a larger field, rounding half up.
static inline int UI_CupPoints_ForPosition(const int *retailPoints, int playerCount, int botCount, int position)
{
	int racerCount = playerCount + botCount;
	if (racerCount <= 0 || position < 0 || position >= racerCount)
	{
		return 0;
	}

	if (botCount == 0)
	{
		return position < UI_CUP_POINTS_RETAIL_AWARD_POSITIONS ? playerCount - position - 1 : 0;
	}

	if (racerCount <= UI_CUP_POINTS_RETAIL_RACERS)
	{
		return retailPoints[position];
	}

	int denominator = racerCount - 1;
	int scaledPosition = position * (UI_CUP_POINTS_RETAIL_RACERS - 1);
	int index = scaledPosition / denominator;
	int remainder = scaledPosition % denominator;
	if (index == UI_CUP_POINTS_RETAIL_RACERS - 1)
	{
		return retailPoints[index];
	}

	int weightedPoints = retailPoints[index] * (denominator - remainder) + retailPoints[index + 1] * remainder;
	return (weightedPoints + denominator / 2) / denominator;
}

#endif
