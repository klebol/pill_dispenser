/*
 * PCF8563.h
 *
 *  Created on: 15 kwi 2022
 *      Author: Michal
 */

#ifndef PCF8563_H_
#define PCF8563_H_

#define RTC_WRITE_ADDR 0xA2
#define RTC_READ_ADDR 0xA3



extern uint8_t PCF_error;		//variable for PCF error storage
#define PCF_VL 0				// if 0 bit of PCF_errer is one, PCF has declared that reliable clock/calendar data is no longer guaranteed
#define PCF_CLKOUT 1			// if 1 bit of PCF_error is one, bad frequency coded value was passed to rtc_set_clkout


typedef struct PCF_Date{
	uint8_t second;
	uint8_t minute;
	uint8_t hour;
	uint8_t day;
	uint8_t weekday;
	uint8_t month;
	uint16_t year;
	int16_t day_minutes;
} date_PCF;

extern date_PCF default_time;
extern date_PCF live_time;
date_PCF new_time;


void rtc_write(uint8_t addr, uint8_t *buff, uint8_t leng);
void rtc_read(uint8_t addr, uint8_t *buff, uint8_t leng);
void rtc_set_date(date_PCF *input_date);
void rtc_read_date(date_PCF *output_date);
void rtc_set_clkout (uint8_t freq);							//3 = 1Hz , 2 = 32Hz, 1 = 1024Hz, 0 = 32,768 kHz
void rtc_set_default_time(uint8_t sec, uint8_t min,uint8_t hour, uint8_t day, uint8_t weekday, uint8_t month,uint16_t year);



#endif /* PCF8563_H_ */
