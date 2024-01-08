/*
 * Disasterbot.h
 *
 * Created: 07/11/2023 11:07:32
 *  Author: pjlys
 */ 


#ifndef DISASTERBOT_H
#define DISASTERBOT_H

#include <avr/sleep.h>
#include <stdbool.h>
#include "MotionTask.h"

/**
 * \brief Initialization task
*/
void InitLineSleepTask(void);
void InitSqSleepTask(void);
void ClearSleepFlag(void);
bool GetSleepFlag(void);

#endif