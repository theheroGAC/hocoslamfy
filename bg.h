#ifndef _BG_H_
#define _BG_H_

#include <stdint.h>

#include "game.h"

#define BG_LAYER_COUNT  8

#define BG_SPEED_1     (-FIELD_SCROLL * 1 / 8)
#define BG_SPEED_2     (-FIELD_SCROLL * 1 / 6)
#define BG_SPEED_3     (-FIELD_SCROLL * 1 / 4)
#define BG_SPEED_4     (-FIELD_SCROLL * 1 / 3)
#define BG_SPEED_5     (-FIELD_SCROLL * 1 / 2)

extern void AdvanceBackground(uint32_t Milliseconds);
extern void DrawBackground(void);

#endif
