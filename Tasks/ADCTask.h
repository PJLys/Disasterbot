#ifndef ADCTASK_H
#define ADCTASK_h

/**
 * Linebot ADC task. Using the ADC driver, reads the 3 line follower sensors and potentiometer at a rate of 100Hz
 * \file AdcTask.h
 * \brief Linebot Power management driver
*/

#include <stdint.h>

typedef struct 
{
	int16_t PhotoL;
	int16_t PhotoM;
	int16_t PhotoR;
	int16_t Potmeter;
} ADCStruct;

/**
 * \brief Initialize analog sensor data acquisition task
*/
void InitADCTask(void);

/**
 * \brief Fetch last analog sensor data set
 * \return: ADCStruct containing sensor data
*/
ADCStruct GetADCData(void);


#endif