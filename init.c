#include <stdbool.h>
#include <stddef.h>

#include "SDL.h"
#include "SDL_image.h"

#include "main.h"
#include "init.h"
#include "audio.h"
#include "platform.h"
#include "title.h"

static const char* BackgroundImageNames[BG_LAYER_COUNT] = {
	"Sky.png",
	"Mountains.png",
	"Clouds3.png",
	"Clouds2.png",
	"Clouds1.png",
	"Grass3.png",
	"Grass2.png",
	"Grass1.png"
};

static const char* TitleScreenFrameNames[TITLE_FRAME_COUNT] = {
	"TitleHeader1.png",
	"TitleHeader2.png",
	"TitleHeader3.png",
	"TitleHeader4.png",
	"TitleHeader5.png",
	"TitleHeader6.png",
	"TitleHeader7.png",
	"TitleHeader8.png"
};

static SDL_Surface* LoadImage(const char* Path)
{
	char path[256];
#ifdef PSVITA
	snprintf(path, sizeof(path), "app0:/data/%s", Path);
	SDL_Surface* res = IMG_Load(path);
	if (res) return res;
	snprintf(path, sizeof(path), "app0:data/%s", Path);
	res = IMG_Load(path);
	if (res) return res;
	snprintf(path, sizeof(path), "data/%s", Path);
	res = IMG_Load(path);
	if (res) return res;
	return NULL;
#else
	snprintf(path, sizeof(path), DATA_PATH "%s", Path);
	return IMG_Load(path);
#endif
}

static bool CheckImage(bool* Continue, bool* Error, const SDL_Surface* Image, const char* Name)
{
	if (Image == NULL)
	{
		*Continue = false;
		*Error = true;
		return false;
	}
	return true;
}

static SDL_Surface* ConvertSurface(bool* Continue, bool* Error, SDL_Surface* Source, const char* Name)
{
	SDL_Surface* Dest;
#ifdef PSVITA
	Dest = SDL_ConvertSurface(Source, Screen->format, SDL_SWSURFACE);
	if (Dest != NULL)
	{
		Dest->flags &= ~SDL_HWACCEL;
		if (Source->format->Amask != 0)
		{
			SDL_SetAlpha(Dest, SDL_SRCALPHA, SDL_ALPHA_OPAQUE);
		}
	}
#else
	if (Source->format->Amask != 0 || (Source->flags & SDL_SRCCOLORKEY))
		Dest = SDL_DisplayFormatAlpha(Source);
	else
		Dest = SDL_DisplayFormat(Source);
#endif
	if (Dest == NULL)
	{
		*Continue = false;
		*Error = true;
		SDL_ClearError();
		return NULL;
	}
	SDL_FreeSurface(Source);
	return Dest;
}

void Initialize(bool* Continue, bool* Error)
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0)
	{
		*Continue = false;
		*Error = true;
		SDL_ClearError();
		return;
	}

#ifdef PSVITA
	RealScreen = SDL_SetVideoMode(960, 544, 16, SDL_HWSURFACE | SDL_DOUBLEBUF);
	if (RealScreen == NULL)
	{
		*Continue = false;
		*Error = true;
		SDL_ClearError();
		return;
	}

	Screen = SDL_CreateRGBSurface(SDL_SWSURFACE, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
		0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	if (Screen == NULL)
	{
		*Continue = false;
		*Error = true;
		return;
	}
#elif defined(OPK)
	Screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 16, SDL_HWSURFACE | SDL_DOUBLEBUF);
	if (Screen == NULL)
	{
		*Continue = false;
		*Error = true;
		SDL_ClearError();
		return;
	}
#else
	Screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_HWSURFACE |
#ifdef SDL_TRIPLEBUF
		SDL_TRIPLEBUF
#else
		SDL_DOUBLEBUF
#endif
		);
	if (Screen == NULL)
	{
		*Continue = false;
		*Error = true;
		SDL_ClearError();
		return;
	}
#endif

	InitializePlatform();
	SDL_ShowCursor(0);

	uint32_t i;
	for (i = 0; i < BG_LAYER_COUNT; i++)
	{
		BackgroundImages[i] = LoadImage(BackgroundImageNames[i]);
		if (!CheckImage(Continue, Error, BackgroundImages[i], BackgroundImageNames[i]))
			return;
		if ((BackgroundImages[i] = ConvertSurface(Continue, Error, BackgroundImages[i], BackgroundImageNames[i])) == NULL)
			return;
	}

	for (i = 0; i < TITLE_FRAME_COUNT; i++)
	{
		TitleScreenFrames[i] = LoadImage(TitleScreenFrameNames[i]);
		if (!CheckImage(Continue, Error, TitleScreenFrames[i], TitleScreenFrameNames[i]))
			return;
		if ((TitleScreenFrames[i] = ConvertSurface(Continue, Error, TitleScreenFrames[i], TitleScreenFrameNames[i])) == NULL)
			return;
	}

	CharacterFrames = LoadImage("Bee.png");
	if (!CheckImage(Continue, Error, CharacterFrames, "Bee.png"))
		return;
	if ((CharacterFrames = ConvertSurface(Continue, Error, CharacterFrames, "Bee.png")) == NULL)
		return;

	CollisionImage = LoadImage("Crash.png");
	if (!CheckImage(Continue, Error, CollisionImage, "Crash.png"))
		return;
	if ((CollisionImage = ConvertSurface(Continue, Error, CollisionImage, "Crash.png")) == NULL)
		return;

	ColumnImage = LoadImage("Bamboo.png");
	if (!CheckImage(Continue, Error, ColumnImage, "Bamboo.png"))
		return;
	if ((ColumnImage = ConvertSurface(Continue, Error, ColumnImage, "Bamboo.png")) == NULL)
		return;

	GameOverFrame = LoadImage("GameOverHeader.png");
	if (!CheckImage(Continue, Error, GameOverFrame, "GameOverHeader.png"))
		return;
	if ((GameOverFrame = ConvertSurface(Continue, Error, GameOverFrame, "GameOverHeader.png")) == NULL)
		return;

	if (!InitializeAudio())
	{
		*Continue = false;
		*Error = true;
		return;
	}
	else
		StartBGM();

	ToTitleScreen();
}

void Finalize(void)
{
	uint32_t i;
	StopBGM();
	FinalizeAudio();
	for (i = 0; i < BG_LAYER_COUNT; i++)
	{
		SDL_FreeSurface(BackgroundImages[i]);
		BackgroundImages[i] = NULL;
	}
	for (i = 0; i < TITLE_FRAME_COUNT; i++)
	{
		SDL_FreeSurface(TitleScreenFrames[i]);
		TitleScreenFrames[i] = NULL;
	}
	SDL_FreeSurface(CharacterFrames);
	CharacterFrames = NULL;
	SDL_FreeSurface(ColumnImage);
	ColumnImage = NULL;
	SDL_FreeSurface(GameOverFrame);
	GameOverFrame = NULL;
#ifdef PSVITA
	if (Screen)
	{
		SDL_FreeSurface(Screen);
		Screen = NULL;
	}
#endif
	SDL_Quit();
}
