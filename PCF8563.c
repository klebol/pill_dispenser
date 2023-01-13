/*
 * PCF8563.c
 *
 *  Created on: 15 kwi 2022
 *      Author: Michal
 */




#include <avr/io.h>
#include <util/delay.h>
#include "TWI.h"
#include "PCF8563.h"

uint8_t PCF_error;

date_PCF default_time;
date_PCF live_time;

void rtc_write(uint8_t addr, uint8_t *buff, uint8_t leng){
	I2C_start();
	I2C_write(RTC_WRITE_ADDR);
	I2C_write(addr);

	while(leng){
		I2C_write(*buff);
		leng--;
		buff++;
	}

	I2C_stop();
}

void rtc_read(uint8_t addr, uint8_t *buff, uint8_t leng){
	I2C_start();
	I2C_write(RTC_WRITE_ADDR);
	I2C_write(addr);
	I2C_stop();
	I2C_start();
	I2C_write(RTC_READ_ADDR);

	while(leng > 1){
		*buff = I2C_read(ACK);
		leng--;
		buff++;
	}
	*buff = I2C_read(NACK);
	I2C_stop();
}

uint8_t dec2bcd(uint8_t dec){
	return ((dec / 10)<<4) | (dec % 10);


}
void rtc_set_date(date_PCF *input_date){
	uint8_t buffer[7];
	buffer[0] = dec2bcd(input_date->second) & 0x7F;
	buffer[1] = dec2bcd(input_date->minute) & 0x7F;
	buffer[2] = dec2bcd(input_date->hour) & 0x3F;
	buffer[3] = dec2bcd(input_date->day) & 0x3F;
	buffer[4] = input_date->weekday & 0x07;
	buffer[5] = dec2bcd(input_date->month) & 0x1F;
	if (input_date->year < 2000) buffer[5] |= 0x80;
	uint8_t year_buf = (input_date->year % 1000);	//year is 16 bit so we had to fit it in dec2bcd function, rtc stores only two last numbers of year in this register
	buffer[6] = dec2bcd(year_buf);

	rtc_write(0x02,buffer,7);



}

void rtc_read_date(date_PCF *output_date){
	uint8_t buffer[7];
	rtc_read(0x02,buffer,7);
	output_date->second = (((buffer[0] >> 4) & 0x07) * 10) + (buffer[0] & 0x0F);
	output_date->minute = (((buffer[1] >> 4) & 0x07) * 10) + (buffer[1] & 0x0F);
	output_date->hour = (((buffer[2] >> 4) & 0x03) * 10) + (buffer[2] & 0x0F);
	output_date->day = (((buffer[3] >> 4) & 0x03) * 10) + (buffer[3] & 0x0F);
	output_date->weekday = buffer[4] & 0x07;
	output_date->month = (((buffer[5] >> 4) & 0x01) * 10) + (buffer[5] & 0x0F);
	output_date->year = ((buffer[6] >> 4) * 10) + (buffer[6] & 0x0F) + 1900;
	if (!(buffer[5] & 0x80)) output_date->year += 100;	//century read (1900 or 2000)
	if(buffer[0] & 0x80) PCF_error |= (1<<PCF_VL);		//low rtc voltage error detection


}

void rtc_set_clkout (uint8_t freq){
	if (freq > 0x03) PCF_error |= (1<<PCF_CLKOUT);
	else{
		uint8_t f_data[1];
		f_data[0] = 0x80 | freq;
		rtc_write(0x0D,f_data,1);
	}
}

void rtc_set_default_time(uint8_t sec, uint8_t min,uint8_t hour, uint8_t day, uint8_t weekday, uint8_t month,uint16_t year){
	default_time.second = sec;
	default_time.minute = min;
	default_time.hour = hour;
	default_time.day = day;
	default_time.weekday = weekday;
	default_time.month = month;
	default_time.year = year;
	rtc_set_date(&default_time);
}
