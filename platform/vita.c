#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "SDL.h"
#include "init.h"
#include "platform.h"

#ifdef PSVITA
#include <psp2/touch.h>
#include <psp2/io/stat.h>
#endif

#ifdef PSVITA
unsigned int _newlib_heap_size_user = 64 * 1024 * 1024;
unsigned int sceLibcHeapSize = 64 * 1024 * 1024;
#endif

#define VITA_BUTTON_TRIANGLE 0
#define VITA_BUTTON_CROSS    2
#define VITA_BUTTON_SELECT   10
#define VITA_BUTTON_START   11
#define VITA_BUTTON_UP     4
#define VITA_BUTTON_DOWN   5
#define VITA_BUTTON_LEFT   6
#define VITA_BUTTON_RIGHT  7

SDL_Surface* RealScreen = NULL;
static Uint32 LastTicks = 0;
static bool FourThree = false;
static bool TouchEnabled = true;
static SDL_Joystick* VitaJoystick = NULL;

#define REAL_SCREEN_WIDTH  960
#define REAL_SCREEN_HEIGHT 544
#define FOUR_THREE_WIDTH   720
#define FOUR_THREE_HEIGHT  540

static uint16_t XMapWide[REAL_SCREEN_WIDTH];
static uint16_t XMapFourThree[FOUR_THREE_WIDTH];
static uint16_t YMapWide[REAL_SCREEN_HEIGHT];
static uint16_t YMapFourThree[FOUR_THREE_HEIGHT];
static uint16_t ConvertedPixels[SCREEN_WIDTH * SCREEN_HEIGHT];
static bool ScaleMapsReady = false;

static void BuildScaleMaps(void)
{
	int x;
	int y;

	for (x = 0; x < REAL_SCREEN_WIDTH; x++)
		XMapWide[x] = (uint16_t) (x * SCREEN_WIDTH / REAL_SCREEN_WIDTH);
	for (x = 0; x < FOUR_THREE_WIDTH; x++)
		XMapFourThree[x] = (uint16_t) (x * SCREEN_WIDTH / FOUR_THREE_WIDTH);
	for (y = 0; y < REAL_SCREEN_HEIGHT; y++)
		YMapWide[y] = (uint16_t) (y * SCREEN_HEIGHT / REAL_SCREEN_HEIGHT);
	for (y = 0; y < FOUR_THREE_HEIGHT; y++)
		YMapFourThree[y] = (uint16_t) (y * SCREEN_HEIGHT / FOUR_THREE_HEIGHT);

	ScaleMapsReady = true;
}

static void ConvertScreenPixels(SDL_Surface* screen)
{
	int x;
	int y;
	const uint32_t* source;
	for (y = 0; y < SCREEN_HEIGHT; y++)
	{
		source = (const uint32_t*) ((const uint8_t*) screen->pixels + y * screen->pitch);
		for (x = 0; x < SCREEN_WIDTH; x++)
		{
			uint32_t p = source[x];
			uint8_t r = (uint8_t) ((p >> 16) & 0xFF);
			uint8_t g = (uint8_t) ((p >> 8) & 0xFF);
			uint8_t b = (uint8_t) (p & 0xFF);
			ConvertedPixels[y * SCREEN_WIDTH + x] = (uint16_t) (((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
		}
	}
}
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
		fprintf(File, "%d %d\n", FourThree ? 1 : 0, TouchEnabled ? 1 : 0);
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
	if (File != NULL)
	{
		if (fscanf(File, "%d %d", &FourThreeValue, &TouchValue) == 2)
		{
			FourThree = FourThreeValue != 0;
			TouchEnabled = TouchValue != 0;
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

	BuildScaleMaps();
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

		uint32_t real_pitch = RealScreen->pitch;
		uint8_t* real_base = (uint8_t*) RealScreen->pixels;
		int viewport_x = FourThree ? 120 : 0;
		int viewport_width = FourThree ? FOUR_THREE_WIDTH : REAL_SCREEN_WIDTH;
		int viewport_height = FourThree ? FOUR_THREE_HEIGHT : REAL_SCREEN_HEIGHT;
		const uint16_t* x_map = FourThree ? XMapFourThree : XMapWide;
		const uint16_t* y_map = FourThree ? YMapFourThree : YMapWide;
		int y;

		if (!ScaleMapsReady)
			BuildScaleMaps();
		ConvertScreenPixels(screen);
		for (y = 0; y < viewport_height; y++)
		{
			uint16_t* dst_row = (uint16_t*) (real_base + y * real_pitch);
			const uint16_t* src_row = ConvertedPixels + y_map[y] * SCREEN_WIDTH;
			int x;
			if (FourThree)
			{
				memset(dst_row, 0, viewport_x * sizeof(uint16_t));
				memset(dst_row + viewport_x + viewport_width, 0,
					(REAL_SCREEN_WIDTH - viewport_x - viewport_width) * sizeof(uint16_t));
			}
			for (x = 0; x < viewport_width; x++)
				dst_row[viewport_x + x] = src_row[x_map[x]];
		}
		for (y = viewport_height; y < REAL_SCREEN_HEIGHT; y++)
			memset(real_base + y * real_pitch, 0, REAL_SCREEN_WIDTH * sizeof(uint16_t));

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

bool IsSelectEvent(const SDL_Event* event)
{
	return IsVitaButtonEvent(event, SDL_JOYBUTTONDOWN, VITA_BUTTON_SELECT);
}

bool IsExitGameEvent(const SDL_Event* event)
{
	if (event->type == SDL_QUIT && SDL_GetTicks() > 1000)
		return true;
	return IsVitaButtonEvent(event, SDL_JOYBUTTONDOWN, VITA_BUTTON_TRIANGLE);
}

const char* GetExitGamePrompt(void)
{
	return "Triangle";
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
