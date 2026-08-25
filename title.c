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

static char*    WelcomeMessage    = NULL;

static uint32_t HeaderFrame       = 0;
static Uint32   HeaderFrameTime   = 0;
static bool     OptionsOpen       = false;
static uint32_t OptionsIndex      = 0;

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

static void ChangeTitleOption(void)
{
	if (OptionsIndex == 0)
		PlatformSetFourThree(!PlatformIsFourThree());
	else if (OptionsIndex == 1)
		PlatformSetTouchEnabled(!PlatformIsTouchEnabled());
	else if (OptionsIndex == 2)
		PlatformSetCircleEnabled(!PlatformIsCircleEnabled());
	if (WelcomeMessage != NULL)
	{
		free(WelcomeMessage);
		WelcomeMessage = NULL;
	}
	ToTitleScreen();
}

static void SelectTitleOption(int Direction)
{
	if (Direction < 0 && OptionsIndex > 0)
		OptionsIndex--;
	else if (Direction > 0 && OptionsIndex < 2)
		OptionsIndex++;
}

void TitleScreenGatherInput(bool* Continue)
{
	SDL_Event ev;
	int TouchX;
	int TouchY;
	if (PlatformSelectPressed())
	{
		OptionsOpen = !OptionsOpen;
		if (OptionsOpen)
			OptionsIndex = 0;
		return;
	}

	if (PlatformEnterPressed())
	{
		if (OptionsOpen)
			ChangeTitleOption();
		else
		{
			ToGame();
			if (WelcomeMessage != NULL)
			{
				free(WelcomeMessage);
				WelcomeMessage = NULL;
			}
		}
		return;
	}

	if (PlatformTouchPressed(&TouchX, &TouchY))
	{
		if (OptionsOpen)
		{
			int InternalY = TouchY * SCREEN_HEIGHT / 544;
			if (InternalY >= 52 && InternalY < 100)
			{
				OptionsIndex = (InternalY - 52) / 16;
				ChangeTitleOption();
			}
		}
		else
		{
			ToGame();
		}
		return;
	}

	{
		int Navigation = PlatformNavigation();
		if (OptionsOpen && Navigation != 0)
		{
			if (Navigation == -1)
				SelectTitleOption(-1);
			else if (Navigation == 1)
				SelectTitleOption(1);
			else if (Navigation == -2 || Navigation == 2)
				ChangeTitleOption();
		}
	}

	while (SDL_PollEvent(&ev))
	{
		if (OptionsOpen)
		{
			if (IsUpEvent(&ev))
				SelectTitleOption(-1);
			else if (IsDownEvent(&ev))
				SelectTitleOption(1);
			else if (IsLeftEvent(&ev) || IsRightEvent(&ev))
				ChangeTitleOption();
			else if (IsExitGameEvent(&ev))
				*Continue = false;
			return;
		}
		if (IsExitGameEvent(&ev))
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

	if (OptionsOpen)
	{
		char OptionsMessage[256];
		SDL_Rect OptionsRect = { .x = 24, .y = 20, .w = 272, .h = 200 };
		const char* ScreenMode = PlatformIsFourThree() ? "4:3" : "16:9";
		const char* TouchMode = PlatformIsTouchEnabled() ? "ON" : "OFF";
		const char* CircleMode = PlatformIsCircleEnabled() ? "ON" : "OFF";
		snprintf(OptionsMessage, sizeof(OptionsMessage), "OPTIONS\n\n%s Screen: %s\n%s Touch: %s\n%s Circle: %s\n\nUp/Down Select\nLeft/Right/Cross Change\nSelect Close", OptionsIndex == 0 ? ">" : " ", ScreenMode, OptionsIndex == 1 ? ">" : " ", TouchMode, OptionsIndex == 2 ? ">" : " ", CircleMode);
		SDL_FillRect(Screen, &OptionsRect, SDL_MapRGB(Screen->format, 0, 0, 0));
		if (SDL_MUSTLOCK(Screen))
			SDL_LockSurface(Screen);
		PrintStringOutline(OptionsMessage,
			SDL_MapRGB(Screen->format, 255, 255, 255),
			SDL_MapRGB(Screen->format, 0, 0, 0),
			Screen->pixels,
			Screen->pitch,
			OptionsRect.x,
			OptionsRect.y,
			OptionsRect.w,
			OptionsRect.h,
			LEFT,
			TOP);
		if (SDL_MUSTLOCK(Screen))
			SDL_UnlockSurface(Screen);
	}

	PlatformFlip(Screen);
}

void ToTitleScreen(void)
{
	if (WelcomeMessage == NULL)
	{
		int Length = 2, NewLength;
		WelcomeMessage = malloc(Length);
		while ((NewLength = PlatformIsCircleEnabled()
			? snprintf(WelcomeMessage, Length, "Press %s to play\nor Circle to exit\n\nIn-game:\n%s to rise\n%s to pause\nSelect to options", GetEnterGamePrompt(), GetBoostPrompt(), GetPausePrompt())
			: snprintf(WelcomeMessage, Length, "Press %s to play\n\nIn-game:\n%s to rise\n%s to pause\nSelect to options", GetEnterGamePrompt(), GetBoostPrompt(), GetPausePrompt())) >= Length)
		{
			Length = NewLength + 1;
			WelcomeMessage = realloc(WelcomeMessage, Length);
		}
	}

	GatherInput = TitleScreenGatherInput;
	DoLogic     = TitleScreenDoLogic;
	OutputFrame = TitleScreenOutputFrame;
}
