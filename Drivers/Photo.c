/*
 * Photo.c
 *
 * Created: 12/10/2023 21:17:52
 *  Author: pjlys
 */ 

void DriverPhotoInit(void) {
	PORTA.DIRCLR = 0b00000111;
	PORTA.PIN0CTRL = 
	PORTA.PIN1CTRL =
	PORTA.PIN2CTRL =
}

uint8_t DriverPhotoGet(void) {
	return 0b000;
}
