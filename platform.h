#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include <stdbool.h>
#include <stdint.h>
#include "SDL.h"

#ifdef PSVITA
extern SDL_Surface* RealScreen;
#endif

void InitializePlatform(void);
void PlatformFlip(SDL_Surface* screen);
Uint32 ToNextFrame(void);

bool IsEnterGamePressingEvent(const SDL_Event* event);
bool IsEnterGameReleasingEvent(const SDL_Event* event);
bool PlatformEnterPressed(void);
bool PlatformSelectPressed(void);
const char* GetEnterGamePrompt(void);

bool IsExitGameEvent(const SDL_Event* event);
const char* GetExitGamePrompt(void);

bool IsSelectEvent(const SDL_Event* event);
bool IsUpEvent(const SDL_Event* event);
bool IsDownEvent(const SDL_Event* event);
bool IsLeftEvent(const SDL_Event* event);
bool IsRightEvent(const SDL_Event* event);
int PlatformNavigation(void);
bool PlatformTouchPressed(int* x, int* y);

bool PlatformIsFourThree(void);
void PlatformSetFourThree(bool enabled);
bool PlatformIsTouchEnabled(void);
void PlatformSetTouchEnabled(bool enabled);

bool IsBoostEvent(const SDL_Event* event);
const char* GetBoostPrompt(void);

bool IsPauseEvent(const SDL_Event* event);
const char* GetPausePrompt(void);

#endif
