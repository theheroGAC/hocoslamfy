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
const char* GetEnterGamePrompt(void);

bool IsExitGameEvent(const SDL_Event* event);
const char* GetExitGamePrompt(void);

bool IsBoostEvent(const SDL_Event* event);
const char* GetBoostPrompt(void);

bool IsPauseEvent(const SDL_Event* event);
const char* GetPausePrompt(void);

#endif
