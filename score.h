#ifndef _SCORE_H_
#define _SCORE_H_

#include <stdbool.h>

enum GameOverReason
{
	FIELD_BORDER_COLLISION,
	RECTANGLE_COLLISION
};

extern void ToScore(uint32_t Score, enum GameOverReason GameOverReason, uint32_t HighScore);
extern void SaveHighScore(uint32_t Score);
extern uint32_t GetHighScore(void);

#endif
