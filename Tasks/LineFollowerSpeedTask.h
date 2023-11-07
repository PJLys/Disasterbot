#ifndef LINEFOLLOWERSPEEDTASK_H
#define LINEFOLLOWERSPEEDTASK_H

/**
 * Linebot line follower task. Follows surface line using encoder based motor speed controller at at specified speed
 * \file LineFollowerDirectTask.h
 * \brief Linebot line follower task
*/

#include <stdint.h>


/**
 * \brief Initialize constant speed mode line follower task.
*/
void InitLineFollowerSpeedTask();

/**
 * \brief Start constant speed mode line follower. The robot will immediately start following the line.
 * \param Speed: average robot speed (encoder counts / s)
*/
void StartLineFollower(float Speed);

/**
 * \brief Stop constant speed mode line follower. The robot will immediately stop.
*/
void StopLineFollower();


#endif