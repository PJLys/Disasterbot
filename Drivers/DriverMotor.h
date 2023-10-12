/**
 * Linebot Motor driver
 * \file DriverLed.h
 * \brief Linebot Motor driver
*/

#include <stdint.h>

///Encoder info struct
typedef struct
{
	int16_t Cnt1;   ///<Encoder 1 pulse count
	int16_t Cnt2;   ///<Encoder 2 pulse count
} EncoderStruct;

/**
 * \brief Initialize Motor driver
*/
void DriverMotorInit(void);


/**
 * \brief Set Motor state
 * \param MotorLeft: Left motor speed and direction. -4095 to + 4095
 * \param MotorRight: Right motor speed and direction. -4095 to + 4095
*/
void DriverMotorSet(int16_t MotorLeft,int16_t MotorRight);


/**
 * \brief Get encoder angular position measurement. Function does not block but will always return the last measured values
 * \return EncoderStruct containing the encoder measurement
*/
EncoderStruct DriverMotorGetEncoder(void);