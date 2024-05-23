#ifndef __MEASURE__H__
#define __MEASURE__H__

#include "main.h"

extern uint8_t method;
extern mode m;

double MeasureFreq_main();
double MeasureFreq_Period();
double MeasureFreq_Freq();
double MeasureGap();
double MeasureHigh();
uint32_t get_timer_frequency();

#endif
