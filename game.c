#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include "SDL.h"
#include "SDL_image.h"

#include "main.h"
#include "init.h"
#include "platform.h"
#include "game.h"
#include "score.h"
#include "bg.h"
#include "text.h"
#include "audio.h"

static uint32_t               Score;

static bool                   Boost;
static bool                   Pause;
static bool                   OptionsOpen;
static bool                   OptionsWasPaused;
static uint32_t               OptionsIndex;
static enum PlayerStatus      PlayerStatus;

static float                  PlayerX;
static float                  PlayerY;

static float                  PlayerSpeed;

static uint8_t                PlayerFrame;

static uint32_t               PlayerFrameTime;

static bool                   PlayerBlinking;

static uint32_t               PlayerBlinkTime;

static enum GameOverReason    GameOverReason;

static struct HocoslamfyRect* Rectangles     = NULL;
static uint32_t               RectangleCount = 0;

static float                  GenDistance;

static void SetOptionsOpen(bool Open)
{
	if (Open == OptionsOpen)
		return;
	if (Open)
	{
		OptionsWasPaused = Pause;
		Pause = true;
	}
	else
	{
		Pause = OptionsWasPaused;
	}
	OptionsOpen = Open;
}

static void ChangeOption(int Direction)
{
	if (OptionsIndex == 0)
		PlatformSetFourThree(!PlatformIsFourThree());
	else if (OptionsIndex == 1)
		PlatformSetTouchEnabled(!PlatformIsTouchEnabled());
	else if (OptionsIndex == 2)
		PlatformSetCircleEnabled(!PlatformIsCircleEnabled());
	(void) Direction;
}

static void SelectOption(int Direction)
{
	if (Direction < 0 && OptionsIndex > 0)
		OptionsIndex--;
	else if (Direction > 0 && OptionsIndex < 2)
		OptionsIndex++;
}

void GameGatherInput(bool* Continue)
{
	SDL_Event ev;
	int TouchX;
	int TouchY;

	{
		int Navigation = PlatformNavigation();
		if (OptionsOpen && Navigation != 0)
		{
			if (Navigation == -1)
				SelectOption(-1);
			else if (Navigation == 1)
				SelectOption(1);
			else if (Navigation == -2 || Navigation == 2)
				ChangeOption(0);
		}
	}

	while (SDL_PollEvent(&ev))
	{
		if (!OptionsOpen && IsSelectEvent(&ev))
		{
			if (PlayerStatus == ALIVE)
			{
				OptionsIndex = 0;
				SetOptionsOpen(true);
			}
		}
		else if (OptionsOpen)
		{
			if (IsSelectEvent(&ev) || IsCircleEvent(&ev))
				SetOptionsOpen(false);
			else if (IsUpEvent(&ev))
				SelectOption(-1);
			else if (IsDownEvent(&ev))
				SelectOption(1);
			else if (IsLeftEvent(&ev) || IsRightEvent(&ev) || IsBoostEvent(&ev))
				ChangeOption(0);
		}
		else if (IsBoostEvent(&ev) && !Pause)
			Boost = true;
		else if (IsPauseEvent(&ev) && PlayerStatus == ALIVE)
			Pause = !Pause;
		else if (IsExitGameEvent(&ev))
		{
			*Continue = false;
			return;
		}
	}

	if (!OptionsOpen && !Pause && PlatformTouchPressed(&TouchX, &TouchY))
		Boost = true;
	if (OptionsOpen && PlatformTouchPressed(&TouchX, &TouchY))
	{
		int InternalY = TouchY * SCREEN_HEIGHT / 544;
		if (InternalY >= 52 && InternalY < 100)
		{
			OptionsIndex = (InternalY - 52) / 16;
			ChangeOption(0);
		}
	}
}

static void SetStatus(const enum PlayerStatus NewStatus)
{
	PlayerFrameTime = 0;
	if (NewStatus == COLLIDED && PlayerStatus != COLLIDED)
		PlaySFXCollision();
	PlayerStatus = NewStatus;
	if (NewStatus == DYING)
		PlayerSpeed = 0.0f;
}

static void AnimationControl(Uint32 Milliseconds)
{
	Uint32 Remainder = Milliseconds;
	switch (PlayerStatus)
	{
		case ALIVE:
		case DYING:

			Remainder = Remainder % (ANIMATION_TIME * ANIMATION_FRAMES);

			PlayerFrame = (PlayerFrame + (PlayerFrameTime + Remainder) / ANIMATION_TIME) % ANIMATION_FRAMES;

			PlayerFrameTime = (PlayerFrameTime + Remainder) % ANIMATION_TIME;
			break;

		case COLLIDED:
			PlayerFrameTime += Remainder;
			if (PlayerFrameTime > COLLISION_TIME)
				SetStatus(DYING);
			break;
	}

	Remainder = Milliseconds;
	while (Remainder > 0)
	{
		if (PlayerBlinking)
		{
			if (PlayerBlinkTime + Remainder >= BLINK_TIME)
			{
				Remainder -= BLINK_TIME - PlayerBlinkTime;
				PlayerBlinking = false;
				PlayerBlinkTime = NONBLINK_TIME_MIN + rand() % (NONBLINK_TIME_MAX - NONBLINK_TIME_MIN);
			}
			else
			{
				PlayerBlinkTime += Remainder;
				Remainder = 0;
			}
		}
		else
		{
			if (Remainder >= PlayerBlinkTime)
			{
				Remainder -= PlayerBlinkTime;
				PlayerBlinking = true;
				PlayerBlinkTime = 0;
			}
			else
			{
				PlayerBlinkTime -= Remainder;
				Remainder = 0;
			}
		}
	}
}

void GameDoLogic(bool* Continue, bool* Error, Uint32 Milliseconds)
{
	if (!Pause && PlayerStatus == ALIVE)
	{
		bool PointAwarded = false;
		uint32_t Millisecond;
		for (Millisecond = 0; Millisecond < Milliseconds; Millisecond++)
		{

			int32_t i;
			for (i = RectangleCount - 1; i >= 0; i--)
			{
				Rectangles[i].Left += FIELD_SCROLL / 1000;
				Rectangles[i].Right += FIELD_SCROLL / 1000;

				if (!Rectangles[i].Passed
				 && Rectangles[i].Right < PlayerX)
				{
					Rectangles[i].Passed = true;
					if (!PointAwarded)
					{
						Score++;
						PointAwarded = true;
						PlaySFXPass();
					}
				}

				if (Rectangles[i].Right < 0.0f)
				{
					memmove(&Rectangles[i], &Rectangles[i + 1], (RectangleCount - i) * sizeof(struct HocoslamfyRect));
					RectangleCount--;
				}
			}

			if (RectangleCount == 0 || FIELD_WIDTH - Rectangles[RectangleCount - 1].Right >= GenDistance)
			{
				float Left;
				if (RectangleCount == 0)
					Left = FIELD_WIDTH + FIELD_SCROLL / 1000;
				else
				{
					Left = Rectangles[RectangleCount - 1].Right + GenDistance;
					GenDistance += RECT_GEN_SPEED;
					if (GenDistance < RECT_GEN_MIN)
						GenDistance = RECT_GEN_MIN;
				}
				Rectangles = realloc(Rectangles, (RectangleCount + 2) * sizeof(struct HocoslamfyRect));
				RectangleCount += 2;
				Rectangles[RectangleCount - 2].Passed = Rectangles[RectangleCount - 1].Passed = false;
				Rectangles[RectangleCount - 2].Left = Rectangles[RectangleCount - 1].Left = Left;
				Rectangles[RectangleCount - 2].Right = Rectangles[RectangleCount - 1].Right = Left + RECT_WIDTH;

				float GapTop = GAP_HEIGHT + (FIELD_HEIGHT / 16.0f) + ((float) rand() / (float) RAND_MAX) * (FIELD_HEIGHT - GAP_HEIGHT - (FIELD_HEIGHT / 8.0f));
				Rectangles[RectangleCount - 2].Top = FIELD_HEIGHT;
				Rectangles[RectangleCount - 2].Bottom = GapTop;
				Rectangles[RectangleCount - 1].Top = GapTop - GAP_HEIGHT;
				Rectangles[RectangleCount - 1].Bottom = 0.0f;
				Rectangles[RectangleCount - 2].Frame = rand() % 3;
				Rectangles[RectangleCount - 1].Frame = rand() % 3;
			}

			PlayerSpeed += GRAVITY / 1000;
			if (Boost)
			{

				PlayerSpeed = SPEED_BOOST;
				Boost = false;
				PlaySFXFly();
			}

			PlayerY += PlayerSpeed / 1000;
			if (PlayerY + (COLLISION_B_HEIGHT / 2) > FIELD_HEIGHT || PlayerY - (COLLISION_B_HEIGHT / 2) < 0.0f)
			{
				SetStatus(COLLIDED);
				GameOverReason = FIELD_BORDER_COLLISION;
				break;
			}

			for (i = 0; i < RectangleCount; i++)
			{
				if ((((PlayerY + (COLLISION_A_HEIGHT / 2) > Rectangles[i].Bottom
				    && PlayerY + (COLLISION_A_HEIGHT / 2) < Rectangles[i].Top)
				   || (PlayerY - (COLLISION_A_HEIGHT / 2) > Rectangles[i].Bottom
				    && PlayerY - (COLLISION_A_HEIGHT / 2) < Rectangles[i].Top))
				  && ((PlayerX - (COLLISION_A_WIDTH  / 2) > Rectangles[i].Left
				    && PlayerX - (COLLISION_A_WIDTH  / 2) < Rectangles[i].Right)
				   || (PlayerX + (COLLISION_A_WIDTH  / 2) > Rectangles[i].Left
				    && PlayerX + (COLLISION_A_WIDTH  / 2) < Rectangles[i].Right)))
				 || (((PlayerY + (COLLISION_B_HEIGHT / 2) > Rectangles[i].Bottom
				    && PlayerY + (COLLISION_B_HEIGHT / 2) < Rectangles[i].Top)
				   || (PlayerY - (COLLISION_B_HEIGHT / 2) > Rectangles[i].Bottom
				    && PlayerY - (COLLISION_B_HEIGHT / 2) < Rectangles[i].Top))
				  && ((PlayerX - (COLLISION_B_WIDTH  / 2) > Rectangles[i].Left
				    && PlayerX - (COLLISION_B_WIDTH  / 2) < Rectangles[i].Right)
				   || (PlayerX + (COLLISION_B_WIDTH  / 2) > Rectangles[i].Left
				    && PlayerX + (COLLISION_B_WIDTH  / 2) < Rectangles[i].Right))))
				{
					SetStatus(COLLIDED);
					GameOverReason = RECTANGLE_COLLISION;
					break;
				}
			}
		}

		AdvanceBackground(Milliseconds);
	}
	else if (PlayerStatus == DYING)
	{
		uint32_t Millisecond;
		for (Millisecond = 0; Millisecond < Milliseconds; Millisecond++)
		{

			PlayerSpeed += GRAVITY / 1000;

			PlayerY += PlayerSpeed / 1000;
			if (PlayerY < 0.0f)
			{
				uint32_t HighScore = GetHighScore();

				ToScore(Score, GameOverReason, HighScore);

				if (Score > HighScore)
					SaveHighScore(Score);
				return;
			}
		}
	}

	AnimationControl(Milliseconds);
}

void GameOutputFrame()
{

	DrawBackground();

	uint32_t i;
	for (i = 0; i < RectangleCount; i++)
	{
		SDL_Rect ColumnDestRect = {
			.x = (int) (Rectangles[i].Left * SCREEN_WIDTH / FIELD_WIDTH) - 20,
			.y = SCREEN_HEIGHT - (int) (Rectangles[i].Top * SCREEN_HEIGHT / FIELD_HEIGHT),
			.w = (int) ((Rectangles[i].Right - Rectangles[i].Left) * SCREEN_WIDTH / FIELD_WIDTH) + 40,
			.h = (int) ((Rectangles[i].Top - Rectangles[i].Bottom) * SCREEN_HEIGHT / FIELD_HEIGHT)
		};
		SDL_Rect ColumnSourceRect = { .x = 0, .y = 0, .w = ColumnDestRect.w, .h = ColumnDestRect.h };

		if (i & 1) {
			ColumnSourceRect.y = 0;
		} else {
			ColumnSourceRect.y = 480 - ColumnDestRect.h;
		}
		ColumnSourceRect.x = 64 * Rectangles[i].Frame;
		SDL_BlitSurface(ColumnImage, &ColumnSourceRect, Screen, &ColumnDestRect);
	}

	uint32_t PassedCount = 0;
	for (i = 0; i < RectangleCount; i += 2)
	{
		if (Rectangles[i].Passed)
			PassedCount++;
	}

	uint32_t RectScore = Score - PassedCount;
	if (SDL_MUSTLOCK(Screen))
		SDL_LockSurface(Screen);
	for (i = 0; i < RectangleCount; i += 2)
	{
		RectScore++;
		char RectScoreString[11];
		sprintf(RectScoreString, "%" PRIu32, RectScore);
		uint32_t RenderedWidth = GetRenderedWidth(RectScoreString) + 2;
		int32_t Left = (int32_t) (((Rectangles[i].Left + Rectangles[i].Right) / 2) * SCREEN_WIDTH / FIELD_WIDTH) - RenderedWidth / 2;

		if (Left >= 0 && Left + RenderedWidth < SCREEN_WIDTH)
		{
			Uint32 RectScoreColor;
			if (Rectangles[i].Passed)
				RectScoreColor = SDL_MapRGB(Screen->format, 64, 255, 64);
			else
				RectScoreColor = SDL_MapRGB(Screen->format, 255, 255, 255);
			PrintStringOutline(RectScoreString,
				RectScoreColor,
				SDL_MapRGB(Screen->format, 0, 0, 0),
				Screen->pixels,
				Screen->pitch,
				Left,

				SCREEN_HEIGHT - (int) (Rectangles[i].Bottom * SCREEN_HEIGHT / FIELD_HEIGHT),
				RenderedWidth,
				(int) (GAP_HEIGHT * SCREEN_HEIGHT / FIELD_HEIGHT),
				CENTER,
				MIDDLE);
		}
	}
	if (SDL_MUSTLOCK(Screen))
		SDL_UnlockSurface(Screen);

	SDL_Rect PlayerDestRect = {
		.x = (int) (PlayerX * SCREEN_WIDTH / FIELD_WIDTH) - (PLAYER_FRAME_SIZE / 2),
		.y = (int) (SCREEN_HEIGHT - (PlayerY * SCREEN_HEIGHT / FIELD_HEIGHT)) - (PLAYER_FRAME_SIZE / 2),
		.w = (int) PLAYER_FRAME_SIZE,
		.h = (int) PLAYER_FRAME_SIZE
	};
	SDL_Rect PlayerSourceRect = {
		.x = 0,
		.y = 0,
		.w = 32,
		.h = 32
	};
#ifdef DRAW_BEE_COLLISION
	SDL_Rect PlayerPixelsA = {
		.x = (int) ((PlayerX - (COLLISION_A_WIDTH / 2)) * SCREEN_WIDTH / FIELD_WIDTH),
		.y = (int) (SCREEN_HEIGHT - ((PlayerY + (COLLISION_A_HEIGHT / 2)) * SCREEN_HEIGHT / FIELD_HEIGHT)),
		.w = (int) (COLLISION_A_WIDTH * SCREEN_HEIGHT / FIELD_HEIGHT),
		.h = (int) (COLLISION_A_HEIGHT * SCREEN_HEIGHT / FIELD_HEIGHT)
	};
	SDL_Rect PlayerPixelsB = {
		.x = (int) ((PlayerX - (COLLISION_B_WIDTH / 2)) * SCREEN_WIDTH / FIELD_WIDTH),
		.y = (int) (SCREEN_HEIGHT - ((PlayerY + (COLLISION_B_HEIGHT / 2)) * SCREEN_HEIGHT / FIELD_HEIGHT)),
		.w = (int) (COLLISION_B_WIDTH * SCREEN_HEIGHT / FIELD_HEIGHT),
		.h = (int) (COLLISION_B_HEIGHT * SCREEN_HEIGHT / FIELD_HEIGHT)
	};
#endif
	switch (PlayerStatus)
	{
		case ALIVE:
			if (PlayerSpeed > -2.0f) {
				PlayerSourceRect.x = 32 * PlayerFrame;
			} else {
				PlayerSourceRect.x = 128 + 32 * PlayerFrame;
			}
			if (PlayerBlinking)
				PlayerSourceRect.x += 64;
			SDL_BlitSurface(CharacterFrames, &PlayerSourceRect, Screen, &PlayerDestRect);
#ifdef DRAW_BEE_COLLISION
			SDL_FillRect(Screen, &PlayerPixelsA, SDL_MapRGB(Screen->format, 255, 255, 255));
			SDL_FillRect(Screen, &PlayerPixelsB, SDL_MapRGB(Screen->format, 255, 255, 255));
#endif
			break;

		case COLLIDED:
			PlayerSourceRect.w = 48;
			PlayerSourceRect.h = 48;
			PlayerDestRect.x -= 8;
			PlayerDestRect.y -= 8;
			PlayerDestRect.w += 16;
			PlayerDestRect.h += 16;
			SDL_BlitSurface(CollisionImage, &PlayerSourceRect, Screen, &PlayerDestRect);
			break;

		case DYING:
			PlayerSourceRect.x = 256 + 32 * PlayerFrame;
			SDL_BlitSurface(CharacterFrames, &PlayerSourceRect, Screen, &PlayerDestRect);
			break;
	}

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

void ToGame(void)
{
	Score = 0;
	Boost = false;
	Pause = false;
	OptionsOpen = false;
	OptionsWasPaused = false;
	OptionsIndex = 0;
	SetStatus(ALIVE);
	PlayerX = FIELD_WIDTH / 4;
	PlayerY = FIELD_HEIGHT / 2;
	PlayerSpeed = 0.0f;

	PlayerFrame = 0;
	PlayerFrameTime = 0;
	PlayerBlinking = true;
	PlayerBlinkTime = 0;

	if (Rectangles != NULL)
	{
		free(Rectangles);
		Rectangles = NULL;
	}
	RectangleCount = 0;
	GenDistance = RECT_GEN_START;

	GatherInput = GameGatherInput;
	DoLogic     = GameDoLogic;
	OutputFrame = GameOutputFrame;
}
