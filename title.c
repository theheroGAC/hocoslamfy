#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "SDL.h"
#include "SDL_image.h"

#include "main.h"
#include "init.h"
#include "platform.h"
#include "game.h"
#include "title.h"
#include "bg.h"
#include "text.h"

static bool     WaitingForRelease = false;
static char*    WelcomeMessage    = NULL;

static uint32_t HeaderFrame       = 0;
static Uint32   HeaderFrameTime   = 0;

static const uint32_t HeaderFrameAnimation[TITLE_ANIMATION_FRAMES] = {
	0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
	0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
	0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 2, 3,
	0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
	0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 2, 3,
	0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
	4, 5, 4, 5, 4, 5, 4, 5, 4, 5, 4, 5,
	4, 5, 4, 5, 4, 5, 4, 5, 4, 5, 4, 5,
	4, 5, 4, 5, 4, 5, 4, 5, 4, 5, 6, 7,
	4, 5, 4, 5, 4, 5, 4, 5, 4, 5, 4, 5,
	4, 5, 4, 5, 4, 5, 4, 5, 4, 5, 6, 7,
	4, 5, 4, 5, 4, 5, 4, 5, 4, 5, 4, 5,
};

void TitleScreenGatherInput(bool* Continue)
{
	SDL_Event ev;

	while (SDL_PollEvent(&ev))
	{
		if (IsEnterGamePressingEvent(&ev))
			WaitingForRelease = true;
		else if (IsEnterGameReleasingEvent(&ev))
		{
			WaitingForRelease = false;
			ToGame();
			if (WelcomeMessage != NULL)
			{
				free(WelcomeMessage);
				WelcomeMessage = NULL;
			}
			return;
		}
		else if (IsExitGameEvent(&ev))
		{
			*Continue = false;
			if (WelcomeMessage != NULL)
			{
				free(WelcomeMessage);
				WelcomeMessage = NULL;
			}
			return;
		}
	}
}

static void AnimationControl(Uint32 Milliseconds)
{
	Uint32 Remainder = Milliseconds;

	Remainder = Remainder % (TITLE_FRAME_TIME * TITLE_ANIMATION_FRAMES);

	HeaderFrame = (HeaderFrame + (HeaderFrameTime + Remainder) / TITLE_FRAME_TIME) % TITLE_ANIMATION_FRAMES;

	HeaderFrameTime = (HeaderFrameTime + Remainder) % TITLE_FRAME_TIME;
}

void TitleScreenDoLogic(bool* Continue, bool* Error, Uint32 Milliseconds)
{
	AnimationControl(Milliseconds);
	AdvanceBackground(Milliseconds);
}

void TitleScreenOutputFrame()
{
	DrawBackground();

	SDL_Rect HeaderDestRect = {
		.x = (SCREEN_WIDTH - TitleScreenFrames[0]->w) / 2,
		.y = ((SCREEN_HEIGHT / 4) - TitleScreenFrames[0]->h) / 2,
		.w = TitleScreenFrames[0]->w,
		.h = TitleScreenFrames[0]->h
	};
	SDL_Rect HeaderSourceRect = {
		.x = 0,
		.y = 0,
		.w = TitleScreenFrames[0]->w,
		.h = TitleScreenFrames[0]->h
	};
	SDL_BlitSurface(TitleScreenFrames[HeaderFrameAnimation[HeaderFrame]], &HeaderSourceRect, Screen, &HeaderDestRect);

	if (SDL_MUSTLOCK(Screen))
		SDL_LockSurface(Screen);
	PrintStringOutline(WelcomeMessage,
		SDL_MapRGB(Screen->format, 255, 255, 255),
		SDL_MapRGB(Screen->format, 0, 0, 0),
		Screen->pixels,
		Screen->pitch,
		0,
		SCREEN_HEIGHT / 4,
		SCREEN_WIDTH,
		SCREEN_HEIGHT - (SCREEN_HEIGHT / 4),
		CENTER,
		MIDDLE);
	if (SDL_MUSTLOCK(Screen))
		SDL_UnlockSurface(Screen);

	PlatformFlip(Screen);
}

void ToTitleScreen(void)
{
	if (WelcomeMessage == NULL)
	{
		int Length = 2, NewLength;
		WelcomeMessage = malloc(Length);
		while ((NewLength = snprintf(WelcomeMessage, Length, "Press %s to play\nor %s to exit\n\nIn-game:\n%s to rise\n%s to pause\n%s to exit", GetEnterGamePrompt(), GetExitGamePrompt(), GetBoostPrompt(), GetPausePrompt(), GetExitGamePrompt())) >= Length)
		{
			Length = NewLength + 1;
			WelcomeMessage = realloc(WelcomeMessage, Length);
		}
	}

	GatherInput = TitleScreenGatherInput;
	DoLogic     = TitleScreenDoLogic;
	OutputFrame = TitleScreenOutputFrame;
}
