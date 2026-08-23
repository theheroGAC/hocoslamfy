#include <stdbool.h>
#include <stddef.h>

#include "SDL.h"
#include "SDL_image.h"

#include "main.h"
#include "init.h"
#include "platform.h"

static bool Continue = true;
static bool Error = false;

SDL_Surface* Screen = NULL;
SDL_Surface* TitleScreenFrames[TITLE_FRAME_COUNT] = { NULL };
SDL_Surface* BackgroundImages[BG_LAYER_COUNT] = { NULL };
SDL_Surface* CharacterFrames = NULL;
SDL_Surface* ColumnImage = NULL;
SDL_Surface* CollisionImage = NULL;
SDL_Surface* GameOverFrame = NULL;

TGatherInput GatherInput;
TDoLogic DoLogic;
TOutputFrame OutputFrame;

int main(int argc, char* argv[])
{
	Initialize(&Continue, &Error);
	Uint32 Duration = 16;
	while (Continue)
	{
		GatherInput(&Continue);
		if (!Continue)
			break;
		DoLogic(&Continue, &Error, Duration);
		if (!Continue)
			break;
		OutputFrame();
		Duration = ToNextFrame();
	}
	Finalize();
	return Error ? 1 : 0;
}
