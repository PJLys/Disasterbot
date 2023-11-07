#ifndef MOTOR_POS_TASK_H
#define MOTOR_POS_TASK_H

/**
 * Linebot motion controller support task. Enables angular positional control of the drive motors. Internally used by MotionTask
 * \file MotorPosTask.h
 * \brief Linebot drive motor angular positional controller
*/


#include <stdint.h>

/**
 * \brief Initialize motor positional controller task
*/
void InitMotorPosTask(void);

/**
 * \brief Increase motor setpoints by a certain number of encoder counts
 * \param IncSet1: left motor setpoint increment (number of encouder counts/100)
 * \param IncSet2: right motor setpoint increment (number of encouder counts/100)
*/
void MotorPosIncSet(int16_t IncSet1,int16_t IncSet2);


/**
 * \brief Enable motor positional controller
*/
void EnableMotorPosTask();

/**
 * \brief Disable motor positional controller
*/
void DisableMotorPosTask();

#endif