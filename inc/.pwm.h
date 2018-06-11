#include <stdlib.h>
#include "stm32f4xx.h"
#include <math.h>

#ifndef SPARK_PWM
#define SPARK_PWM

#ifndef PI
#define PI (3.141592653589793)
#endif

#ifndef TWOPI
#define TWOPI (6.28318530718)
#endif

#ifndef PIFIFTY
#define PIFIFTY (0.06283185307)
#endif

typedef struct audio_struct {
  int *frequency;
  unsigned char *volume;
  float *cosTable;
} AUDIO_T;

int initAudio(void);

float* initMyCos(void);

void setFrequency(int *freq);

void setVolume(unsigned char *vol);

void destroyAudio(void);

float myCos(float x);

#endif
