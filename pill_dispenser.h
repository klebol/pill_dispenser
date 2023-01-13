/*
 * pill_dispenser.h
 *
 *  Created on: 29 maj 2022
 *      Author: Michal
 */

#ifndef PILL_DISPENSER_H_
#define PILL_DISPENSER_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdlib.h>
#include "lcd.h"
#include "PCF8563.h"
#include "StepperMotor.h"

#define same_everyday 0
#define different_everyweekday 1


#define max_dose_number 16


enum weekday {sun,mon,tue,wed,thu,fri,sat};
enum weekday day;

uint8_t dosage_mode;
uint8_t dose_numb [7]; 	//doses number each day (nr.0-sun , nr.6-sat)
uint8_t all_doses;

uint8_t minute_flag;
uint8_t latest_minute;

uint8_t its_dosetime_flag;
uint8_t doses_definied;

struct dose_time{
	uint8_t weekday;
	uint8_t hour;
	uint8_t minute;
	int16_t day_minutes;
	int16_t minutes_to;
};
typedef struct dose_time D_time;

D_time Dose_times [max_dose_number];

D_time *next_dose;


uint8_t d_define_doses(void);
void watch_dose_times(void);
D_time *d_find_next_dose(void);
D_time *find_next_dose(void);
uint8_t d_dosesearch (uint8_t weekday, uint8_t hour, uint8_t minute);
uint8_t find_next_dose_number(void);
uint8_t take_dose(void);
uint8_t d_infill_medicine (void);
uint8_t change_live_time(void);
uint8_t d_display_doses(void);
void d_main_home_display(void);
uint8_t home_stepper(void);
void key_confirm1(void);
void key_confirm2(void);
void key_confirm3(void);
void d_view_doses(void);
void time_check(void);

#endif /* PILL_DISPENSER_H_ */
