/*
 * main.c
 *
 *  Created on: 5 kwi 2022
 *      Author: Michal
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdlib.h>
#include "servo9g.h"
#include "lcd.h"
#include "PCF8563.h"
#include "StepperMotor.h"
#include "pill_dispenser.h"
#include "hardware_layer.h"

int main(void){
	Stepper_init();
	servo_init();
	I2C_init();
	lcd_init(LCD_DISP_ON);
	Reg_init();
	uint8_t i;
	for(i = 0; i<=max_dose_number; i++){
		Dose_times[i].weekday = 255;
	}
	rtc_set_default_time(0,30,15,20,mon,6,2022);
	lcd_clrscr();
	OCR2A = 0;
	while(1){
		Debounce(&PINC,key1,0,0,key_confirm1,key_confirm1);
		Debounce(&PINC,key2,0,0,key_confirm2,key_confirm2);
		Debounce(&PINC,key3,0,0,key_confirm3,key_confirm3);
		time_check();
		Stepper();
		d_main_home_display();
		if(!(PINB & stop_switch)) switch_push = 1;
		else switch_push = 0;
		}
}

