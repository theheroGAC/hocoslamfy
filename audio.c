#include <stdlib.h>
#include <stdbool.h>

#include "SDL.h"
#include "SDL_mixer.h"

#include "init.h"

static bool       SND_Available = false;

static Mix_Music* BGM           = NULL;

static Mix_Chunk* SFX_Fly       = NULL;
static Mix_Chunk* SFX_Pass      = NULL;
static Mix_Chunk* SFX_Collision = NULL;
static Mix_Chunk* SFX_HighScore = NULL;

static Mix_Chunk* LoadSFX(const char* Path)
{
	char fullpath[256];
#ifdef PSVITA
	snprintf(fullpath, sizeof(fullpath), "app0:/data/%s", Path);
	Mix_Chunk* Result = Mix_LoadWAV(fullpath);
	if (!Result) {
		snprintf(fullpath, sizeof(fullpath), "app0:data/%s", Path);
		Result = Mix_LoadWAV(fullpath);
	}
	if (!Result) {
		snprintf(fullpath, sizeof(fullpath), "data/%s", Path);
		Result = Mix_LoadWAV(fullpath);
	}
#else
	snprintf(fullpath, sizeof(fullpath), DATA_PATH "%s", Path);
	Mix_Chunk* Result = Mix_LoadWAV(fullpath);
#endif
	if (Result == NULL)
		printf("%s: Mix_LoadWAV failed: %s\n", Path, Mix_GetError());
	else
		printf("Successfully loaded %s\n", Path);
	return Result;
}

bool InitializeAudio()
{
	if (Mix_OpenAudio(44100, AUDIO_S16SYS, 2 , 1024 ))
	{
		printf("warning: Mix_OpenAudio failed: %s\n", Mix_GetError());
		printf("Sound will not be available.\n");
	}
	else
	{
		printf("Mix_OpenAudio succeeded\n");
		SND_Available = true;
	}

	if (SND_Available)
	{
#ifdef PSVITA
		BGM = Mix_LoadMUS("app0:/data/bgm.wav");
		if (!BGM) BGM = Mix_LoadMUS("app0:data/bgm.wav");
		if (!BGM) BGM = Mix_LoadMUS("data/bgm.wav");
#else
		BGM = Mix_LoadMUS(DATA_PATH "bgm.wav");
#endif
		if (BGM == NULL)
		{
			printf("warning: Mix_LoadMUS failed: %s\n", Mix_GetError());
		}
		else
			printf("Successfully loaded bgm.wav\n");

		SFX_Fly       = LoadSFX("fly.wav");
		SFX_Pass      = LoadSFX("pass.wav");
		SFX_Collision = LoadSFX("collision.wav");
		SFX_HighScore = LoadSFX("highscore.wav");
	}

	return true;
}

void FinalizeAudio()
{
	if (SND_Available)
	{
		Mix_HaltMusic();
		Mix_FreeMusic(BGM);
		BGM = NULL;
		Mix_FreeChunk(SFX_Fly);
		Mix_FreeChunk(SFX_Pass);
		Mix_FreeChunk(SFX_Collision);
		Mix_FreeChunk(SFX_HighScore);
		Mix_CloseAudio();
	}
}

void StartBGM()
{
	if (SND_Available)
	{
		Mix_PlayMusic(BGM, -1 );
	}
}

void StopBGM()
{
	if (SND_Available)
	{
		Mix_HaltMusic();
	}
}

void PlaySFXFly()
{
	if (SND_Available)
	{
		Mix_PlayChannel(-1, SFX_Fly, 0);
	}
}

void PlaySFXPass()
{
	if (SND_Available)
	{
		Mix_PlayChannel(-1, SFX_Pass, 0);
	}
}

void PlaySFXCollision()
{
	if (SND_Available)
	{
		Mix_PlayChannel(-1, SFX_Collision, 0);
	}
}

void PlaySFXHighScore()
{
	if (SND_Available)
	{
		Mix_PlayChannel(-1, SFX_HighScore, 0);
	}
}
