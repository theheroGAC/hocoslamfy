#ifndef _INIT_H_
#define _INIT_H_

#include <stdbool.h>

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

#ifdef PSVITA
#define DATA_PATH "app0:data/"
#elif defined(OPK)
#define DATA_PATH "./"
#else
#define DATA_PATH "./data/"
#endif

void Initialize(bool* Continue, bool* Error);
void Finalize(void);

#endif
