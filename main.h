#ifndef _MAIN_H_
#define _MAIN_H_

#include <stdbool.h>
#include "SDL.h"

#include "title.h"
#include "bg.h"

typedef void (*TGatherInput) (bool* Continue);
typedef void (*TDoLogic) (bool* Continue, bool* Error, Uint32 Milliseconds);
typedef void (*TOutputFrame) (void);

extern SDL_Surface* Screen;
extern SDL_Surface* TitleScreenFrames[TITLE_FRAME_COUNT];
extern SDL_Surface* BackgroundImages[BG_LAYER_COUNT];
extern SDL_Surface* CharacterFrames;
extern SDL_Surface* ColumnImage;
extern SDL_Surface* CollisionImage;
extern SDL_Surface* GameOverFrame;
extern TGatherInput GatherInput;
extern TDoLogic     DoLogic;
extern TOutputFrame OutputFrame;

#endif
