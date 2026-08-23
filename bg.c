#include <math.h>

#include "SDL.h"

#include "main.h"
#include "init.h"
#include "game.h"
#include "bg.h"

static       float    BG_X     [BG_LAYER_COUNT] = {
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f
};

static const float    BG_Speed [BG_LAYER_COUNT] = {
	BG_SPEED_1, BG_SPEED_2, BG_SPEED_3, BG_SPEED_4,
	BG_SPEED_5, BG_SPEED_3, BG_SPEED_4, BG_SPEED_5
};

static const uint32_t BG_StartY[BG_LAYER_COUNT] = {

	 50, 128,  32,  16,
	  0, 180, 190, 204
};
static const uint32_t BG_Height[BG_LAYER_COUNT] = {
	140,  60,  28,  28,
	 32,  20,  28,  36
};

void AdvanceBackground(uint32_t Milliseconds)
{
	uint32_t i;
	for (i = 0; i < BG_LAYER_COUNT; i++)
	{
		BG_X[i] = fmodf(BG_X[i] + BG_Speed[i] * Milliseconds / 1000, FIELD_WIDTH * 0.5f);
	}
}

void DrawBackground(void)
{
	uint32_t i;
	for (i = 0; i < BG_LAYER_COUNT; i++)
	{
		SDL_Rect SourceRect = {
			.x = (int) (BG_X[i] * SCREEN_WIDTH / FIELD_WIDTH),
			.y = 0,
			.w = SCREEN_WIDTH,
			.h = BG_Height[i] };
		SDL_Rect DestRect = {
			.x = 0,
			.y = BG_StartY[i],
			.w = SCREEN_WIDTH,
			.h = BG_Height[i] };
		SDL_BlitSurface(BackgroundImages[i], &SourceRect, Screen, &DestRect);
	}
}
