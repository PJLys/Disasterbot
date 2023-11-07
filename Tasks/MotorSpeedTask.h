#ifndef MOTOR_SPEED_TASK_H
#define MOTOR_SPEED_TASK_H

/**
 * Linebot motion controller support task. Enables angular velocity control of the drive motors. Internally used by MotionTask
 * \file MotorSpeedTask.h
 * \brief Linebot drive motor angular velocity controller
*/

#include <stdint.h>


/**
 * \brief Initialize motor speed controller
*/
void InitMotorSpeedTask(void);

/**
 * \brief Set motor speed
 * \param Speed1: left motor speed (encoder counts / s)
 * \param IncSet2: right motor speed (encoder counts / s)
*/
void MotorSpeedSet(int16_t Speed1,int16_t Speed2);

/**
 * \brief Enable motor speed controller
*/
void EnableMotorSpeedTask();


/**
 * \brief Disable motor speed controller
*/
void DisableMotorSpeedTask();

#endif