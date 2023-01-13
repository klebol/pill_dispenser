/*
 * StepperMotor.c
 *
 *  Created on: 4 maj 2022
 *      Author: Michal
 */

#include <avr/io.h>
#include "StepperMotor.h"
#include "hardware_layer.h"



void OneStep (uint8_t direction){
	static int8_t k = 0;
	if(k>3) k=0;
	if(k<0) k=3;
	switch(k){
	case 0:
		Krok1;
		break;
	case 1:
		Krok2;
		break;
	case 2:
		Krok3;
		break;
	case 3:
		Krok4;
		break;
	default:
		break;
	}
	if (direction) k--;
	else k++;

}

void Stepper (void){
	if(!T2){
		if(step_pos < step_pos_fu){
			OneStep(1);
			step_pos++;
			if(step_pos == step_pos_fu){
				step_pos = 0;
				step_pos_fu = 0;
				StepperOff;
			}
		}
		T2=1;
	}
}

void Stepper_init(void){
	DDRD |= Steppin0 | Steppin1 | Steppin2 | Steppin3;
}

void move_one_compartment(void){
	step_pos_fu = 131;
}


