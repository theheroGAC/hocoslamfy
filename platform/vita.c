#include <stdbool.h>
#include <stdint.h>

#include "SDL.h"
#include "platform.h"

#ifdef PSVITA
unsigned int _newlib_heap_size_user = 64 * 1024 * 1024;
unsigned int sceLibcHeapSize = 64 * 1024 * 1024;
#endif

#define VITA_BUTTON_CROSS   2
#define VITA_BUTTON_CIRCLE  1
#define VITA_BUTTON_SELECT  10
#define VITA_BUTTON_START   11

SDL_Surface* RealScreen = NULL;
static Uint32 LastTicks = 0;

void InitializePlatform(void)
{
	SDL_VITA_SetVideoModeBilinear(1);
	SDL_VITA_SetVideoModeSync(1);

	SDL_JoystickEventState(SDL_ENABLE);
	if (SDL_JoystickOpen(0) == NULL)
	{
		SDL_ClearError();
	}

	SDL_Event flush_ev;
	while (SDL_PollEvent(&flush_ev)) {}

	LastTicks = SDL_GetTicks();
}

void PlatformFlip(SDL_Surface* screen)
{
	if (RealScreen && screen)
	{
		if (SDL_MUSTLOCK(RealScreen))
			SDL_LockSurface(RealScreen);
		if (SDL_MUSTLOCK(screen))
			SDL_LockSurface(screen);

		uint32_t screen_pitch = screen->pitch;
		uint32_t real_pitch = RealScreen->pitch;
		const uint8_t* screen_base = (const uint8_t*) screen->pixels;
		uint8_t* real_base = (uint8_t*) RealScreen->pixels;

		int y;
		for (y = 0; y < 544; y++)
		{
			int src_y = (y * 240) / 544;
			const uint32_t* src_row = (const uint32_t*)(screen_base + src_y * screen_pitch);
			uint16_t* dst_row = (uint16_t*)(real_base + y * real_pitch);

			int x;
			for (x = 0; x < 320; x++)
			{
				uint32_t p = src_row[x];
				uint8_t r = (p >> 16) & 0xFF;
				uint8_t g = (p >> 8) & 0xFF;
				uint8_t b = p & 0xFF;
				uint16_t p16 = (uint16_t)(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));

				dst_row[3 * x + 0] = p16;
				dst_row[3 * x + 1] = p16;
				dst_row[3 * x + 2] = p16;
			}
		}

		if (SDL_MUSTLOCK(screen))
			SDL_UnlockSurface(screen);
		if (SDL_MUSTLOCK(RealScreen))
			SDL_UnlockSurface(RealScreen);

		SDL_Flip(RealScreen);
	}
	else if (screen)
	{
		SDL_Flip(screen);
	}
}

Uint32 ToNextFrame(void)
{
	SDL_Delay(8);
	Uint32 Ticks = SDL_GetTicks();
	Uint32 Duration = Ticks - LastTicks;
	LastTicks = Ticks;
	return Duration;
}

static bool IsVitaButtonEvent(const SDL_Event* event, Uint8 type, Uint8 button)
{
	return event->type == type
	    && event->jbutton.button == button;
}

bool IsEnterGamePressingEvent(const SDL_Event* event)
{
	return IsVitaButtonEvent(event, SDL_JOYBUTTONDOWN, VITA_BUTTON_CROSS)
	    || IsVitaButtonEvent(event, SDL_JOYBUTTONDOWN, VITA_BUTTON_START);
}

bool IsEnterGameReleasingEvent(const SDL_Event* event)
{
	return IsVitaButtonEvent(event, SDL_JOYBUTTONUP, VITA_BUTTON_CROSS)
	    || IsVitaButtonEvent(event, SDL_JOYBUTTONUP, VITA_BUTTON_START);
}

const char* GetEnterGamePrompt(void)
{
	return "Cross/Start";
}

bool IsExitGameEvent(const SDL_Event* event)
{
	if (event->type == SDL_QUIT && SDL_GetTicks() > 1000)
		return true;
	return IsVitaButtonEvent(event, SDL_JOYBUTTONDOWN, VITA_BUTTON_CIRCLE)
	    || IsVitaButtonEvent(event, SDL_JOYBUTTONDOWN, VITA_BUTTON_SELECT);
}

const char* GetExitGamePrompt(void)
{
	return "Circle/Select";
}

bool IsBoostEvent(const SDL_Event* event)
{
	return IsVitaButtonEvent(event, SDL_JOYBUTTONDOWN, VITA_BUTTON_CROSS);
}

const char* GetBoostPrompt(void)
{
	return "Cross";
}

bool IsPauseEvent(const SDL_Event* event)
{
	return IsVitaButtonEvent(event, SDL_JOYBUTTONDOWN, VITA_BUTTON_START);
}

const char* GetPausePrompt(void)
{
	return "Start";
}
