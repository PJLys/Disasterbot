#ifndef MOTION_TASK_H
#define MOTIONN_TASK_H

/**
 * Linebot motion controller. Enables motor encoder feedback based motion commands (drive straight, rotate around center and follow a line segment)
 * \file MotionTask.h
 * \brief Linebot motion controller task
*/

/**
 * \brief Initialize motion controller task
*/
void InitMotionTask(void);

/**
 * \brief Drive in a straight line over a specified distance, with a specified speed. Blocks until complete
 * \param Distance: Distance to travel in mm. Negative distance: backwards movement
 * \param Speed: Speed in mm/s
*/
void DriveStraight(float Distance, float Speed);

/**
 * \brief Drive and follow a line segment until the end, with a specified speed. Blocks until complete
 * \param Speed: Speed in mm/s
*/
void DriveSegment(float Speed);

/**
 * \brief Rotate around center of the robot
 * \param Angle: Angle to rotate (Positive=counterclockwise)
 * \param Speed: Speed in mm/s
*/
void RotateCenter(float Angle, float Speed);

#endif