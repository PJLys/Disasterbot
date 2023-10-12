/*
 * Photo.h
 *
 * Created: 12/10/2023 21:13:08
 *  Author: pjlys
 */ 
#include "hwconfig.h"
#include <stdint.h>

#define PHOTO1 0b001
#define PHOTO2 0b010
#define PHOTO3 0b100


void DriverPhotoInit(void);
uint8_t DriverPhotoGet(void);
