#ifndef GYRO_TASK_H
#define GYRO_TASK_H

/**
 * Linebot gyroscope task. Using the MPU6050 reads Z axis (yaw) gyroscope data, calculates yaw rate in deg/s and integrates to yaw angle.
 * \file GyroTask.h
 * \brief Initialize gyroscope data acquisition task
*/


/**
 * \brief Initialize gyroscope data acquisition task
*/

void InitGyroTask(void);

/**
 * \brief Fetch last gyroscope data
 * \param YawRate: gyroscope yaw rate (°/S)
 * \param Yaw: gyroscope yaw angle (°) 
*/
void GyroGet(float *YawRate, float *Yaw);

#endif