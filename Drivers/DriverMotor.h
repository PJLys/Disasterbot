/**
 * Linebot PWM Motor driver
 * \file DriverMotor.h
 * \brief Linebot Motor driver
*/

#include <stdint.h>

#define ENCODER_EVENT_QUEUE_LENGTH 20

///Encoder info struct
typedef struct
{
	int16_t Cnt1;   ///<Encoder 1 pulse count
	int16_t Cnt2;   ///<Encoder 2 pulse count
} EncoderStruct;

///Encoder event enumeration
typedef enum {NONE,RISING_1A,RISING_1B,RISING_2A,RISING_2B,FALLING_1A,FALLING_1B,FALLING_2A,FALLING_2B} EncoderEventEnum;

///Encoder event struct
typedef struct  
{
	uint32_t Time;			///<Event timestamp
	EncoderEventEnum Event; ///<Event type
} EncoderEventStruct;


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

/**
 * \brief Get encoder event from the event FIFO
 * \return EncoderEventStruct containing the last event. If no events on the FIFO, returns an event with timestamp 0
*/
EncoderEventStruct DriverMotorGetEncoderEvent(void);

/**
 * \brief Reset encoder position measurement to 0
 */
void DriverMotorResetEncoder(void);