#ifndef LINEFOLLOWERDIRECT_TASK_H
#define LINEFOLLOWERDIRECT_TASK_H

/**
 * Linebot line follower task. Follows surface line using direct line follower sensor-->motor PWM feedback
 * \file LineFollowerDirectTask.h
 * \brief Linebot line follower task
*/

/**
 * \brief Initialize direct mode line follower task.
*/
void InitLineFollowerDirectTask(void);

/**
 * \brief Enables direct mode line follower task. The robot will immediately start following a line
*/
void EnableLineFollowerDirectTask();

/**
 * \brief Disables direct mode line follower task. The robot will immediately stop
*/
void DisableLineFollowerDirectTask();

#endif