#ifndef _AUDIO_H_
#define _AUDIO_H_

#include <stdbool.h>

extern bool InitializeAudio();
extern void FinalizeAudio();
extern void StartBGM();
extern void StopBGM();
extern void PlaySFXFly();
extern void PlaySFXPass();
extern void PlaySFXCollision();
extern void PlaySFXHighScore();

#endif
