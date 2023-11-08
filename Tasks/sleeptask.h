/*
 * sleeptask.h
 *
 * Created: 07/11/2023 11:07:32
 *  Author: pjlys
 */ 


#ifndef SLEEP_TASK_H
#define SLEEP_TASK_H

#include <avr/sleep.h>
#include <stdbool.h>

/**
 * \brief Initialization task
*/
void InitSleepTask(void);
void ClearSleepFlag(void);
bool GetSleepFlag(void);

#endif