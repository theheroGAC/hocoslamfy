#ifndef _GAME_H_
#define _GAME_H_

#define SPEED_BOOST      3.41f

#define GRAVITY         -9.78f

#define FIELD_SCROLL    -2.00f

#define RECT_GEN_START   2.00f

#define RECT_GEN_SPEED  -0.01f

#define RECT_GEN_MIN     0.50f

#define RECT_WIDTH       0.42f

#define GAP_HEIGHT       1.33f

#define PLAYER_FRAME_SIZE      32

#define COLLISION_A_WIDTH      0.36f
#define COLLISION_A_HEIGHT     0.18f
#define COLLISION_B_WIDTH      0.16f
#define COLLISION_B_HEIGHT     0.32f

#define ANIMATION_TIME  50

#define ANIMATION_FRAMES 2

#define BLINK_TIME     200

#define NONBLINK_TIME_MIN  800

#define NONBLINK_TIME_MAX 5000

#define COLLISION_TIME 200

#define FIELD_HEIGHT     4.00f

#define FIELD_WIDTH    (SCREEN_WIDTH * (FIELD_HEIGHT / SCREEN_HEIGHT))

struct HocoslamfyRect
{
	float Left;
	float Top;
	float Right;
	float Bottom;
	bool  Passed;
	int   Frame;
};

enum PlayerStatus
{
	ALIVE,
	COLLIDED,
	DYING
};

extern void ToGame(void);

#endif
