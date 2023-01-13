/*
 * servo9g.c
 *
 *  Created on: 10 cze 2022
 *      Author: Michal
 */


#include <avr/io.h>
#include "servo9g.h"

void servo_init(){
	TCCR2A |= (1<<COM2A1); 											//clear OC2A on compare match, set at bottom
	TCCR2A |= (1<<WGM20) | (1<<WGM21); 								 //fast pwm 8 bit
	TCCR2B |= (1<<CS20) | (1<<CS21) | (1<<CS22);  					//prescaler 64
}

void servo_enable(){
	DDRB |= (1<<PB3);								//servo as output
}

void servo_disable(){
	DDRB &= ~(1<<PB3);								//servo as output
}

void servo_set_position(uint8_t pos){
	if(pos > 35) pos = 35;
	OCR2A = pos;
}
