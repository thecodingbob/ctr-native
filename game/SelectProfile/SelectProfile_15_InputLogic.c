#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80048f0c-0x800490c4
u32 SelectProfile_InputLogic(struct RectMenu *menu, s16 numRows, u32 confirmFlags)
{
	u32 handled = 0;
	u32 tap = sdata->buttonTapPerPlayer[0];

	// D-Pad, Cross, Square, Triangle, Circle
	if ((tap & 0x4007f) == 0)
		return 0;

	if ((confirmFlags & 1) == 0)
	{
		u16 oldRow = menu->rowSelected;
		u16 nextRow = oldRow - 2;
		s16 selectedRow;

		if ((tap & BTN_UP) != 0)
		{
			menu->rowSelected = nextRow;
		}
		else
		{
			nextRow = oldRow + 2;

			if ((tap & BTN_DOWN) != 0)
			{
				menu->rowSelected = nextRow;
			}
			else
			{
				nextRow = oldRow ^ 1;

				if ((tap & (BTN_LEFT | BTN_RIGHT)) != 0)
					menu->rowSelected = nextRow;
			}
		}

		selectedRow = menu->rowSelected;
		if (selectedRow < 0)
		{
			menu->rowSelected = 0;
			selectedRow = menu->rowSelected;
		}

		if (numRows <= selectedRow)
			menu->rowSelected = numRows - 1;

		if ((u16)menu->rowSelected != oldRow)
			OtherFX_Play(0, 1);

		if (((tap & (BTN_CROSS | BTN_CIRCLE)) == 0) || ((numRows == 0) && (sdata->memcardAction != 1)))
		{
			if ((tap & (BTN_TRIANGLE | BTN_SQUARE)) != 0)
			{
				OtherFX_Play(2, 1);
				handled = 1;
				menu->rowSelected = -1;
			}
		}
		else
		{
			OtherFX_Play(1, 1);
			handled = 1;

			if (sdata->mcScreenText == MC_SCREEN_WARNING_UNFORMATTED)
				menu->rowSelected = 0;
		}
	}
	else
	{
		u32 cancel = (tap & (BTN_TRIANGLE | BTN_SQUARE)) != 0;

		if (cancel)
		{
			OtherFX_Play(2, 1);
			menu->rowSelected = -1;
		}

		handled = cancel;

		if (((confirmFlags & 2) != 0) && ((tap & (BTN_CROSS | BTN_CIRCLE)) != 0))
		{
			OtherFX_Play(1, 1);
			handled = 1;
		}
	}

	RECTMENU_ClearInput();
	return handled;
}
