#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "SDL.h"
#include "platform.h"

#ifdef PSVITA
#include <psp2/touch.h>
#include <psp2/io/stat.h>
#endif

#ifdef PSVITA
unsigned int _newlib_heap_size_user = 64 * 1024 * 1024;
unsigned int sceLibcHeapSize = 64 * 1024 * 1024;
#endif

#define VITA_BUTTON_CROSS   2
#define VITA_BUTTON_CIRCLE  1
#define VITA_BUTTON_SELECT  10
#define VITA_BUTTON_START   11
#define VITA_BUTTON_UP     4
#define VITA_BUTTON_DOWN   5
#define VITA_BUTTON_LEFT   6
#define VITA_BUTTON_RIGHT  7

SDL_Surface* RealScreen = NULL;
static Uint32 LastTicks = 0;
static bool FourThree = false;
static bool TouchEnabled = true;
static bool CircleEnabled = false;
static SDL_Joystick* VitaJoystick = NULL;
#ifdef PSVITA
static bool TouchActive = false;
static SceInt16 TouchMinX = 0;
static SceInt16 TouchMaxX = 1919;
static SceInt16 TouchMinY = 0;
static SceInt16 TouchMaxY = 1087;
#endif

static void SaveSettings(void)
{
#ifdef PSVITA
	FILE* File;
	sceIoMkdir("ux0:data/hocoslamfy", 0777);
	File = fopen("ux0:data/hocoslamfy/settings", "w");
	if (File != NULL)
	{
		fprintf(File, "%d %d %d\n", FourThree ? 1 : 0, TouchEnabled ? 1 : 0, CircleEnabled ? 1 : 0);
		fclose(File);
	}
#endif
}

static void LoadSettings(void)
{
#ifdef PSVITA
	FILE* File = fopen("ux0:data/hocoslamfy/settings", "r");
	int FourThreeValue;
	int TouchValue;
	int CircleValue;
	if (File != NULL)
	{
		if (fscanf(File, "%d %d %d", &FourThreeValue, &TouchValue, &CircleValue) == 3)
		{
			FourThree = FourThreeValue != 0;
			TouchEnabled = TouchValue != 0;
			CircleEnabled = CircleValue != 0;
		}
		fclose(File);
	}
#endif
}

void InitializePlatform(void)
{
	LoadSettings();
	SDL_VITA_SetVideoModeBilinear(1);
	SDL_VITA_SetVideoModeSync(1);

	SDL_JoystickEventState(SDL_ENABLE);
	VitaJoystick = SDL_JoystickOpen(0);
	if (VitaJoystick == NULL)
	{
		SDL_ClearError();
	}

#ifdef PSVITA
	SceTouchPanelInfo PanelInfo;
	if (sceTouchGetPanelInfo(SCE_TOUCH_PORT_FRONT, &PanelInfo) >= 0)
	{
		TouchMinX = PanelInfo.minAaX;
		TouchMaxX = PanelInfo.maxAaX;
		TouchMinY = PanelInfo.minAaY;
		TouchMaxY = PanelInfo.maxAaY;
	}
	sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_START);
#endif

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
		int viewport_x = FourThree ? 120 : 0;
		int viewport_width = FourThree ? 720 : 960;
		int viewport_height = FourThree ? 540 : 544;
		int y;

		for (y = 0; y < 544; y++)
		{
			uint16_t* dst_row = (uint16_t*)(real_base + y * real_pitch);
			int x;
			for (x = 0; x < 960; x++)
				dst_row[x] = 0;
			if (y < viewport_height)
			{
				int src_y = (y * 240) / viewport_height;
				const uint32_t* src_row = (const uint32_t*)(screen_base + src_y * screen_pitch);
				for (x = 0; x < viewport_width; x++)
				{
					int src_x = (x * 320) / viewport_width;
					uint32_t p = src_row[src_x];
					uint8_t r = (p >> 16) & 0xFF;
					uint8_t g = (p >> 8) & 0xFF;
					uint8_t b = p & 0xFF;
					uint16_t p16 = (uint16_t)(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
					dst_row[viewport_x + x] = p16;
				}
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
	return event->type == type && event->jbutton.button == button;
}

static bool IsVitaHatEvent(const SDL_Event* event, Uint8 direction)
{
	return event->type == SDL_JOYHATMOTION && (event->jhat.value & direction) != 0;
}

static int AxisState[2] = { 0, 0 };
static Uint32 AxisNextRepeat[2] = { 0, 0 };
static bool EnterButtonState = false;
static bool SelectButtonState = false;

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

bool PlatformEnterPressed(void)
{
	bool pressed = false;
	bool current = false;
	if (VitaJoystick != NULL)
	{
		SDL_JoystickUpdate();
		current = SDL_JoystickGetButton(VitaJoystick, VITA_BUTTON_CROSS) != 0
		       || SDL_JoystickGetButton(VitaJoystick, VITA_BUTTON_START) != 0;
	}
	if (current && !EnterButtonState)
		pressed = true;
	EnterButtonState = current;
	return pressed;
}

bool PlatformSelectPressed(void)
{
	bool pressed = false;
	bool current = false;
	if (VitaJoystick != NULL)
	{
		SDL_JoystickUpdate();
		current = SDL_JoystickGetButton(VitaJoystick, VITA_BUTTON_SELECT) != 0;
	}
	if (current && !SelectButtonState)
		pressed = true;
	SelectButtonState = current;
	return pressed;
}

bool IsCircleEvent(const SDL_Event* event)
{
	return CircleEnabled && IsVitaButtonEvent(event, SDL_JOYBUTTONDOWN, VITA_BUTTON_CIRCLE);
}

bool IsSelectEvent(const SDL_Event* event)
{
	return IsVitaButtonEvent(event, SDL_JOYBUTTONDOWN, VITA_BUTTON_SELECT);
}

bool IsExitGameEvent(const SDL_Event* event)
{
	if (event->type == SDL_QUIT && SDL_GetTicks() > 1000)
		return true;
	return IsCircleEvent(event);
}

const char* GetExitGamePrompt(void)
{
	return "Circle";
}

bool IsUpEvent(const SDL_Event* event)
{
	return IsVitaButtonEvent(event, SDL_JOYBUTTONDOWN, VITA_BUTTON_UP)
	    || IsVitaHatEvent(event, SDL_HAT_UP);
}

bool IsDownEvent(const SDL_Event* event)
{
	return IsVitaButtonEvent(event, SDL_JOYBUTTONDOWN, VITA_BUTTON_DOWN)
	    || IsVitaHatEvent(event, SDL_HAT_DOWN);
}

bool IsLeftEvent(const SDL_Event* event)
{
	return IsVitaButtonEvent(event, SDL_JOYBUTTONDOWN, VITA_BUTTON_LEFT)
	    || IsVitaHatEvent(event, SDL_HAT_LEFT);
}

bool IsRightEvent(const SDL_Event* event)
{
	return IsVitaButtonEvent(event, SDL_JOYBUTTONDOWN, VITA_BUTTON_RIGHT)
	    || IsVitaHatEvent(event, SDL_HAT_RIGHT);
}

int PlatformNavigation(void)
{
	int axisX = 0;
	int axisY = 0;
	int state;
	Uint32 now;

	if (VitaJoystick != NULL)
	{
		SDL_JoystickUpdate();
		axisX = SDL_JoystickGetAxis(VitaJoystick, 0);
		axisY = SDL_JoystickGetAxis(VitaJoystick, 1);
	}
	if (axisY < -16384)
		state = -1;
	else if (axisY > 16384)
		state = 1;
	else if (axisX < -16384)
		state = -2;
	else if (axisX > 16384)
		state = 2;
	else
	{
		AxisState[0] = 0;
		AxisState[1] = 0;
		AxisNextRepeat[0] = 0;
		AxisNextRepeat[1] = 0;
		return 0;
	}

	if (state == -1 || state == 1)
	{
		if (AxisState[1] != state)
		{
			AxisState[1] = state;
			AxisNextRepeat[1] = SDL_GetTicks() + 120;
			return state;
		}
		if (SDL_GetTicks() >= AxisNextRepeat[1])
		{
			AxisNextRepeat[1] = SDL_GetTicks() + 75;
			return state;
		}
	}
	else
	{
		if (AxisState[0] != state)
		{
			AxisState[0] = state;
			AxisNextRepeat[0] = SDL_GetTicks() + 120;
			return state;
		}
		if (SDL_GetTicks() >= AxisNextRepeat[0])
		{
			AxisNextRepeat[0] = SDL_GetTicks() + 75;
			return state;
		}
	}
	return 0;
}

bool PlatformTouchPressed(int* x, int* y)
{
#ifdef PSVITA
	SceTouchData TouchData;
	bool Pressed = false;
	if (TouchEnabled && sceTouchPeek(SCE_TOUCH_PORT_FRONT, &TouchData, 1) >= 0 && TouchData.reportNum > 0)
	{
		SceTouchReport* Report = &TouchData.report[0];
		if (!TouchActive)
		{
			Pressed = true;
			if (x != NULL)
				*x = (Report->x - TouchMinX) * 960 / (TouchMaxX - TouchMinX + 1);
			if (y != NULL)
				*y = (Report->y - TouchMinY) * 544 / (TouchMaxY - TouchMinY + 1);
		}
		TouchActive = true;
	}
	else
	{
		TouchActive = false;
	}
	return Pressed;
#else
	(void) x;
	(void) y;
	return false;
#endif
}

bool PlatformIsFourThree(void)
{
	return FourThree;
}

void PlatformSetFourThree(bool enabled)
{
	FourThree = enabled;
	SaveSettings();
}

bool PlatformIsTouchEnabled(void)
{
	return TouchEnabled;
}

void PlatformSetTouchEnabled(bool enabled)
{
	TouchEnabled = enabled;
	SaveSettings();
}

bool PlatformIsCircleEnabled(void)
{
	return CircleEnabled;
}

void PlatformSetCircleEnabled(bool enabled)
{
	CircleEnabled = enabled;
	SaveSettings();
}

bool IsBoostEvent(const SDL_Event* event)
{
	return IsVitaButtonEvent(event, SDL_JOYBUTTONDOWN, VITA_BUTTON_CROSS);
}

const char* GetBoostPrompt(void)
{
	return TouchEnabled ? "Cross/Touch" : "Cross";
}

bool IsPauseEvent(const SDL_Event* event)
{
	return IsVitaButtonEvent(event, SDL_JOYBUTTONDOWN, VITA_BUTTON_START);
}

const char* GetPausePrompt(void)
{
	return "Start";
}
