/*
 * pill_dispenser.c
 *
 *  Created on: 29 maj 2022
 *      Author: Michal
 */


#include "pill_dispenser.h"
#include "hardware_layer.h"
#include "servo9g.h"
#include "StepperMotor.h"

const char days_names [][9] PROGMEM = {
{"Sunday"},
{"Monday"},
{"Tuesday"},
{"Wednesday"},
{"Thursday"},
{"Friday"},
{"Saturday"}
};

void key_confirm1 (void){
	pushed_key = 1;
	LED_SW;
}
void key_confirm2 (void){
	pushed_key = 2;
	LED_SW;
}
void key_confirm3 (void){
	pushed_key = 3;
	LED_SW;
}
void time_check(void){
	if(second_flag){
		rtc_read_date(&live_time);
		if(latest_minute != live_time.minute){
			minute_flag = 1;
			latest_minute = live_time.minute;
		}else minute_flag = 0;
		second_flag = 0;
	}
	if(minute_flag == 1 && doses_definied == 1) watch_dose_times();
}
uint8_t d_dosesearch (uint8_t weekday, uint8_t hour, uint8_t minute){
	int8_t i = 0;
	uint8_t liststart = 0, listend = 0;
	for (i = weekday - 1; i>-1; i--){			//searching for number of first dose data in list (D_time Dose_times) for wanted day. need to use -1 is caused by
		liststart += dose_numb[i];				//fact that arrays (dose_numb in this case) are numbered from 0, not 1, and we want to determinate position of first dose
	}											//by adding number of doses of previous days (not wanted) to dose_numb, so we need -1
	listend = liststart + dose_numb[weekday];  //last data for current day in list
	for(i = liststart; i<listend;i++){
		if(weekday == Dose_times[i].weekday && hour == Dose_times[i].hour && minute == Dose_times[i].minute){
			return 1;
		}
	}
	return 0;
}
uint8_t d_define_doses(void){
	enum CS {display_1,define_dosage_type,display_2,define_dose_number,display_3,define_doses,define_doses_error,infill,finish};	//funcion for defining dosage schedule, built in state-machine design
	static enum CS config_state; 																							//displays have separate states where possible (config_state) to prevent from continuos
	static uint8_t lastp_key = 0, input_value = 0, buffer = 0, hour_buffer = 0, hour_minute_set = 0, day_switch = 0;		//displaying the same lines while being some time in the same state
	static uint8_t defining_dose_number = 0, error_code = 0;																		//(preventing from sending the same data via I2C to oled, that is, from cloggin I2C bus)
	static uint16_t time_minutes_last = 0, time_minutes_curr = 0;

	if(config_state == display_1){
		lcd_clrscr();
		lcd_gotoxy(1,0); lcd_puts("Select dosage type");
		lcd_gotoxy(0,1); lcd_puts("---------------------");
		lcd_gotoxy(0,3); lcd_puts(">Same doses everyday<");
		lcd_gotoxy(3,5); lcd_puts("Different doses");
		lcd_gotoxy(4,6); lcd_puts("every weekday");
		lastp_key = 1;
		config_state = define_dosage_type;
	}
	else if(config_state == define_dosage_type){
		switch(pushed_key){
		case 1:
			if(lastp_key != 1){
				lcd_gotoxy(0,3); lcd_puts(">Same doses everyday<");
				lcd_clearline(5); lcd_clearline(6);
				lcd_gotoxy(3,5); lcd_puts("Different doses");
				lcd_gotoxy(4,6); lcd_puts("every weekday");
				pushed_key = 0; lastp_key = 1;
			}
			break;
		case 2:
			lcd_clrscr();
			if(lastp_key == 1) dosage_mode = same_everyday;
			else if (lastp_key == 3) dosage_mode = different_everyweekday;
			config_state = display_2;
			pushed_key = 0; lastp_key = 0;
			break;
		case 3:
			if(lastp_key != 3){
				lcd_gotoxy(1,5); lcd_puts("> Different doses <");
				lcd_gotoxy(2,6); lcd_puts("> every weekday < ");
				lcd_clearline(3);
				lcd_gotoxy(1,3); lcd_puts("Same doses everyday");
				pushed_key = 0; lastp_key = 3;
			}
			break;
		default:
			break;
		}
	}
	else if(config_state == display_2){
		lcd_gotoxy(2,0); lcd_puts("Give a number of");
		if(dosage_mode == same_everyday){
			lcd_gotoxy(3,1); lcd_puts("doses everyday");
		}
		else if(dosage_mode == different_everyweekday){
			lcd_gotoxy(5,1); lcd_puts("doses on :");
			lcd_gotoxy(6,3); lcd_puts("Sunday");
			lcd_gotoxy(1,7); lcd_puts("0/"); lcd_puti(max_dose_number);
		}
		lcd_charMode(2);
		lcd_gotoxy(9,5); lcd_puts("0");
		config_state = define_dose_number;
	}
	else if(config_state == define_dose_number){
		switch(pushed_key){
		case 1:
			if (input_value > 0) input_value--;
			lcd_gotoxy(9,5); lcd_puti(input_value);
			if( (input_value+1)%10 == 0) lcd_puts(" ");		//clear zero which is left after 10 going to 9
			pushed_key = 0;
			break;
		case 2:
			if(dosage_mode == same_everyday){
				if(input_value == 0){
					error_code = 1;
					lcd_clrscr(); lcd_charMode(2);
					lcd_gotoxy (5,0); lcd_puts("ERROR");
					lcd_charMode(1);
					lcd_gotoxy(0,3); lcd_puts("Number of doses must");
					lcd_gotoxy(3,4); lcd_puts("be bigger than 0");
					lcd_gotoxy(3,6); lcd_puts("Enter different");
					lcd_gotoxy(8,7); lcd_puts("value");
					config_state = define_doses_error;
					pushed_key = 0; input_value = 0;
					break;
				}
				all_doses = input_value;
				dose_numb[0] = input_value;
				lcd_clrscr();
				lcd_charMode(1);
				config_state = display_3;
				pushed_key = 0; input_value = 0;
			}
			else if(dosage_mode == different_everyweekday){
				dose_numb[day_switch] = input_value;
				all_doses += input_value;
				lcd_charMode(1);
				if(all_doses < 10) lcd_gotoxy(1,7);
				else lcd_gotoxy(0,7);
				lcd_puti(all_doses);
				lcd_charMode(2);
				pushed_key = 0; input_value = 0;
				if(day_switch > 5 || all_doses > 15) {						//when number of doses for all days has beed definied or hit maximum
					if(all_doses == 0){
						error_code = 1;
						lcd_clrscr(); lcd_charMode(2);
						lcd_gotoxy (5,0); lcd_puts("ERROR");
						lcd_charMode(1);
						lcd_gotoxy(0,3); lcd_puts("Number of doses must");
						lcd_gotoxy(3,4); lcd_puts("be bigger than 0");
						lcd_gotoxy(3,6); lcd_puts("Enter different");
						lcd_gotoxy(8,7); lcd_puts("value");
						config_state = define_doses_error;
						pushed_key = 0; input_value = 0; day_switch = 0;
						break;
					}
					lcd_charMode(1);
					day_switch = 0;											 //reset day_switch for next function block
					while(dose_numb[day_switch] == 0) day_switch++;			 //find first day with number of doses bigger than zero to avoid defining doses for days with 0 doses
					config_state = display_3;
				}
				else {
					lcd_gotoxy(9,5); lcd_puts("0 ");
					lcd_charMode(1);
					lcd_clearline(3);
					lcd_gotoxy(6,3); lcd_puts_p(days_names[day_switch+1]);
					lcd_charMode(2);
					day_switch++;
				}
			}
			break;
		case 3:
			if (input_value < max_dose_number && all_doses + input_value < max_dose_number) input_value++;
			lcd_gotoxy(9,5); lcd_puti(input_value);
			pushed_key = 0;
			break;
		default:
			break;
		}
	}
	else if(config_state == display_3){
		lcd_clrscr();
		lcd_gotoxy(0,0); lcd_puts("-----Define doses----");
		if(dosage_mode == different_everyweekday){
			lcd_gotoxy(6,1);
			while(dose_numb[day_switch] == 0) day_switch++;			//skip days with 0 doses for defining procedure
			lcd_gotoxy(6,1); lcd_puts_p(days_names[day_switch]);	//display first day for dofine doses
		}
		lcd_gotoxy(3,2); lcd_puts("Dose number: "); lcd_puti(defining_dose_number+1);
		lcd_gotoxy(5,3); lcd_puts("Hour : Minute");
		lcd_charMode(2);
		lcd_gotoxy(5,5); lcd_puts("00:00");
		config_state = define_doses;

	}
	else if(config_state == define_doses){
		switch(pushed_key){
			case 1:															//LEFT KEY, DECREMENT INPUT VALUE
				input_value--;												//decrement input value
				if(hour_minute_set == 0){									//if input value is hour
					lcd_gotoxy(5,5);
					if(input_value == 255) input_value = 23;
				}
				else if(hour_minute_set == 1){								//if input value is minute
					lcd_gotoxy(11,5);
					if(input_value == 255) input_value = 59;
				}
				if(input_value < 10) lcd_puts("0");							//when input value is less than 10, display it as 0x (01, 03, ...)
				lcd_puti(input_value);										//display value
				pushed_key = 0;												//reset pushed_key flag
				break;
			case 2:																			// MIDDLE KEY, INPUT VALUE OPERATIONS / ASSIGNMENTS
				if(dosage_mode == same_everyday){																// FOR MODE - SAME DOSES EVERY DAY
					if(hour_minute_set == 0){
						hour_buffer = input_value;
						hour_minute_set = 1;
					}
					else if(hour_minute_set == 1){
						time_minutes_curr = (hour_buffer * 60) + input_value;
						if(time_minutes_curr < time_minutes_last){
							error_code = 2;
							lcd_clrscr(); lcd_charMode(2);
							lcd_gotoxy (5,0); lcd_puts("ERROR");
							lcd_charMode(1);
							lcd_gotoxy(0,3); lcd_puts("Next dose have to");
							lcd_gotoxy(0,4); lcd_puts("be later than previous");
							lcd_gotoxy(1,6); lcd_puts("Enter different time");
							hour_minute_set = 0;
							input_value = 0; pushed_key = 0;
							config_state = define_doses_error;
							break;
						}
						time_minutes_last = time_minutes_curr;
						if(d_dosesearch(0,hour_buffer,input_value)){
							error_code = 2;
							lcd_clrscr(); lcd_charMode(2);
							lcd_gotoxy (5,0); lcd_puts("ERROR");
							lcd_charMode(1);
							lcd_gotoxy(2,3); lcd_puts("This dose already");
							lcd_gotoxy(8,4); lcd_puts("exist");
							lcd_gotoxy(1,6); lcd_puts("Enter different time");
							hour_minute_set = 0;
							input_value = 0; pushed_key = 0;
							config_state = define_doses_error;
							break;
						}
						Dose_times[defining_dose_number].minute = input_value;
						Dose_times[defining_dose_number].hour = hour_buffer;
						Dose_times[defining_dose_number].weekday = 0;
						defining_dose_number++;
						hour_minute_set = 0;
						if(defining_dose_number == all_doses){

							config_state = finish;
							input_value = 0; pushed_key = 0;
							lcd_charMode(1); lcd_clrscr();
							T2 = 200;
						}
						else{
							lcd_gotoxy(5,5); lcd_puts("00:00");
							lcd_charMode(1);
							lcd_gotoxy(16,2); lcd_puti(defining_dose_number+1);
							lcd_charMode(2);
						}
					}
					input_value = 0; pushed_key = 0;
				}
				if(dosage_mode == different_everyweekday){						// FOR MODE - DIFFERENT DOSES EVERY WEEKDAY
					if(hour_minute_set == 0){									//DEFINE HOUR
						hour_buffer = input_value;								//definig input value as hour of an dose
						hour_minute_set = 1;									//hour_minute_set flag to 1 for defining minute next
					}
					else if(hour_minute_set == 1){								//DEFINE MINUTE
						time_minutes_curr = hour_buffer * 60 + input_value;
						if(time_minutes_curr < time_minutes_last){
							error_code = 2;
							lcd_clrscr(); lcd_charMode(2);
							lcd_gotoxy (5,0); lcd_puts("ERROR");
							lcd_charMode(1);
							lcd_gotoxy(0,3); lcd_puts("Next dose have to");
							lcd_gotoxy(0,4); lcd_puts("be later than previous");
							lcd_gotoxy(1,6); lcd_puts("Enter different time");
							hour_minute_set = 0;
							input_value = 0; pushed_key = 0;
							config_state = define_doses_error;
							break;
						}
						time_minutes_last = time_minutes_curr;
						if(d_dosesearch(day_switch,hour_buffer,input_value)){
							error_code = 2;
							lcd_clrscr(); lcd_charMode(2);
							lcd_gotoxy (5,0); lcd_puts("ERROR");
							lcd_charMode(1);
							lcd_gotoxy(2,3); lcd_puts("This dose already");
							lcd_gotoxy(8,4); lcd_puts("exist");
							lcd_gotoxy(1,6); lcd_puts("Enter different time");
							hour_minute_set = 0;
							input_value = 0; pushed_key = 0;
							config_state = define_doses_error;
							break;
						}
						Dose_times[defining_dose_number].hour = hour_buffer;
						Dose_times[defining_dose_number].minute = input_value;	//defining next input value as minute of an dose
						Dose_times[defining_dose_number].weekday = day_switch;	//assign weekday currently being defined to an dose data

						if(buffer + 1 == dose_numb[day_switch]){				//buffer holds number of dose currently being defined. when buffer reaches a number of doses of specific day,
							day_switch++;										//day_switch is incremented, and buffer is reduced to zero and starts counting to next day dose number
							time_minutes_curr = 0;
							time_minutes_last = 0;
							buffer = 0;
							lcd_charMode(1);
							lcd_gotoxy(6,1); lcd_puts_p(days_names[day_switch]); //display next day
							lcd_charMode(2);
						}else buffer ++;										//incerement buffer after every dose data until it hits number of doses of day currently being defined

						defining_dose_number++;									//incerement number of dose currently being defined (number of dose out of ALL doses, not number of dose out of specific day doses (buffer is for that))
						hour_minute_set = 0;									//flag set to 0, next input value will be hour of next dose

						if(defining_dose_number == all_doses){					//when all doses are definied...
							config_state = finish;
							lcd_charMode(1); lcd_clrscr();
							input_value = 0; pushed_key = 0;
							T2 = 200;
						}
						else{
							lcd_gotoxy(5,5); lcd_puts("00:00");
							lcd_charMode(1);
							lcd_gotoxy(16,2); lcd_puti(defining_dose_number+1);
							lcd_charMode(2);
							input_value = 0; pushed_key = 0;
						}
					}
					input_value = 0; pushed_key = 0;
				}
				break;
			case 3:																//LEFT KEY, INCREMENT INPUT VALUE
				input_value++;
				pushed_key = 0;
				if(hour_minute_set == 0){
					if(input_value == 24) input_value = 0;
					lcd_gotoxy(5,5);
				}
				else if(hour_minute_set == 1){
					if(input_value == 60) input_value = 0;
					lcd_gotoxy(11,5);
				}
				if(input_value < 10) lcd_puts("0");
				lcd_puti(input_value);
				break;
			default:
				break;
		}
	}
	else if(config_state == define_doses_error){
		if(pushed_key == 2) {
			switch(error_code){
			case 1:
				config_state = display_2;
				break;
			case 2:
				config_state = display_3;
				break;
			default:
				break;
			}
			lcd_clrscr();
			error_code = 0;
			pushed_key = 0;
		}
	}
	else if(config_state == finish){
		lcd_gotoxy(2,2); lcd_puts("Doses definied");
		lcd_gotoxy(4,4); lcd_puts("successfully");
		doses_definied = 1;
		watch_dose_times();
		if(!T2){
			config_state = display_1;
			lcd_clrscr();
			return 1;
		}
	}
	return 0;
}

uint8_t d_display_doses (void){
	enum DS {display,wait_for_leave};
	static enum DS display_state;

	switch(display_state){
	case display:
		lcd_clrscr();
		uint8_t i;
		uint8_t column = 0, line = 0;
		for(i = 0; i < all_doses; i++){
			if(line > 7) {
				column = 10;
				line = 0;
			}
			lcd_gotoxy(column,line);
			if(Dose_times[i].hour < 10) lcd_puts("0");
			lcd_puti(Dose_times[i].hour);
			lcd_puts(":");
			if(Dose_times[i].minute < 10) lcd_puts("0");
			lcd_puti(Dose_times[i].minute);
			line++;
		}
		lcd_gotoxy(10,6); lcd_puti(all_doses);
		lcd_gotoxy(10,7); lcd_puti(next_dose->hour); lcd_puts(":"); lcd_puti(next_dose->minute);
		display_state = wait_for_leave;
		break;
	case wait_for_leave:
		if(pushed_key == 2){
			lcd_clrscr();
			display_state = display;
			pushed_key = 0;
			return 1;
		}
	}
	return 0;
}


D_time *find_next_dose(void){								//UNUSED FUNCTION FOR FINDING NEXT DOSE WHEN THEY'RE NOT SORTED
	int8_t i = 0;
	uint8_t liststart = 0, listend = 0;
	uint8_t nextd_search;
	D_time *next_dose;

	for (i = live_time.weekday - 1; i>-1; i--){												//searching for number of first dose data in list (D_time Dose_times) for current day. need to use -1 is caused by
		liststart += dose_numb[i];															//fact that arrays (dose_numb in this case) are numbered from 0, not 1
	}
	listend = liststart + dose_numb[live_time.weekday];  									//last data for current day in list
	live_time.day_minutes = (live_time.hour * 60) + live_time.minute;						//calculate current time in minutes only
	for(i = liststart; i<listend;i++){														//for each dose current day...
		Dose_times[i].day_minutes = (Dose_times[i].hour * 60) + Dose_times[i].minute;		//calculate dose time in minutes
		Dose_times[i].minutes_to = Dose_times[i].day_minutes - live_time.day_minutes;		//calculate time to this dose relative to current time
	}
	nextd_search = liststart;																//first dose on list current day is first iteration in comparation process
	for(i = liststart+1; i<listend;i++){													//compare consecutive dose times for finding the closest one in future
		if(Dose_times[i].minutes_to > 0 && Dose_times[i].minutes_to != live_time.day_minutes && Dose_times[i].minutes_to < Dose_times[nextd_search].minutes_to) nextd_search = i;
	}																						//sought dose have to be in future (minutes_to > 0 and minutes_to != day_minutes) and have to be the closest to current time (comparing with nextd_search)

	if(nextd_search == liststart){
		if(Dose_times[nextd_search].minutes_to < 0 || Dose_times[nextd_search].minutes_to == live_time.day_minutes){
			return 0;
		}
	}
	next_dose = &Dose_times[nextd_search];													//return pointer to next dose
	return next_dose;
}
D_time *d_find_next_dose(void){
	int8_t i = 0;
	uint8_t listend = 0;
	uint8_t nextd_search;
	D_time *next_dose;

	listend = all_doses;
	live_time.day_minutes = (live_time.hour * 60) + live_time.minute;
	for(i = 0; i<listend;i++){
		Dose_times[i].day_minutes = (Dose_times[i].hour * 60) + Dose_times[i].minute;
		Dose_times[i].minutes_to = Dose_times[i].day_minutes - live_time.day_minutes;
	}
	nextd_search = 0;
	for(i = 1; i<listend;i++){
		if(Dose_times[nextd_search].minutes_to <= 0) nextd_search = i;
	}
	if(Dose_times[nextd_search].minutes_to <= 0 || Dose_times[nextd_search].minutes_to == live_time.day_minutes){
			next_dose = &Dose_times[0];
			return next_dose;
	}
	next_dose = &Dose_times[nextd_search];
	return next_dose;
}

uint8_t home_stepper (void){
	enum HS{init,homing,first_comp_goto};
	static enum HS homing_state;
	static uint8_t steps;

	switch(homing_state){
	case init:
		steps = 0;
		lcd_clrscr();
		lcd_charMode(2);
		lcd_gotoxy(1,4); lcd_puts("Homing...");
		lcd_charMode(1);
		homing_state = homing;
		break;
	case homing:
		if(switch_push == 0){
			if(!T3){
			T3 = 1;
			OneStep(0);
			}
		}
		else if(switch_push == 1){
			steps = 80;
			homing_state = first_comp_goto;
		}
		break;
	case first_comp_goto:
		if(steps > 0){
			if(!T3){
			T3 = 1;
			OneStep(1);
			steps--;
			}
		}
		else {
			StepperOff;
			homing_state = init;
			lcd_clrscr();
			return 1;
		}
		break;
	}
	return 0;
}
void watch_dose_times(void){
	enum WS {find,compare,wait_for_dose_take};
	static enum WS watch_state;
	switch(watch_state){
	case find:
		next_dose = d_find_next_dose();
		watch_state = compare;
		break;
	case compare:
		if(next_dose->hour == live_time.hour && next_dose->minute == live_time.minute){
				its_dosetime_flag = 1;
				watch_state = wait_for_dose_take;
			}
		break;
	case wait_for_dose_take:
		if(its_dosetime_flag == 0) watch_state = find;
		break;
	}
}
uint8_t d_infill_medicine (void){
	enum IS {init,homing_1,display,infill,next_compartment,homing_2};
	static enum IS refill_state;
	static D_time *refill_dose;
	static uint8_t compartment_number = 1;

	switch(refill_state){
	case init:
		refill_dose = d_find_next_dose();
		if(refill_dose == 0) refill_dose = &Dose_times[0];
		refill_state = homing_1;
		break;
	case homing_1:
		if(home_stepper()) refill_state = display;
		break;
	case display:
		lcd_gotoxy(0,0); lcd_puts("Infill compartment "); lcd_puti(compartment_number);
		lcd_gotoxy(1,1); lcd_puts("with medicine for");
		lcd_charMode(2);
		lcd_gotoxy(4,4); if(refill_dose->hour < 10) lcd_puts("0"); lcd_puti(refill_dose->hour);
		lcd_puts(":");
		if(refill_dose->minute < 10) lcd_puts("0"); lcd_puti(refill_dose->minute);
		lcd_charMode(1);
		refill_state = infill;
		break;
	case infill:
		if(pushed_key == 2){
			refill_dose++;
			compartment_number++;
			if(refill_dose == &Dose_times[all_doses]) refill_dose = &Dose_times[0];
			refill_state = next_compartment;
			pushed_key = 0;
		}
		if(compartment_number == max_dose_number + 1){
			lcd_clrscr();
			refill_state = 0;
			refill_dose = 0;
			compartment_number = 0;
			refill_state = homing_2;
		}
		break;
	case next_compartment:
		move_one_compartment();
		refill_state = display;
		break;
	case homing_2:
		if(home_stepper()) return 1;
		break;
	}
	return 0;
}

uint8_t change_live_time(void){
	enum PS {init,display_date,day,minute,hour,end};
	static enum PS process_state , next_state;
	static uint8_t input_value, blink, boundary_input;

	switch(pushed_key){
	case 1:
		if(input_value > 0) input_value--;
		else if(input_value == 0) input_value = boundary_input;
		pushed_key = 0;
		break;
	case 2:
		pushed_key = 0;
		if(process_state == day){
			new_time.weekday = input_value;
			next_state = hour;
		}
		else if(process_state == hour){
			new_time.hour = input_value;
			next_state = minute;
		}
		else if(process_state == minute){
			new_time.minute = input_value;
			next_state = end;
		}
		process_state = display_date;
		input_value = 0;
		break;
	case 3:
		if(input_value < boundary_input) input_value++;
		else if(input_value == boundary_input) input_value = 0;
		pushed_key = 0;
		break;
	default:
		break;
	}
	switch(process_state){
	case init:
		new_time.second = 0;
		new_time.minute = 0;
		new_time.hour = 0;
		new_time.day = 0;
		new_time.weekday = 0;
		new_time.month = 6;
		new_time.year = 2022;
		process_state = day;
		break;
	case display_date:
		lcd_charMode(2);
		lcd_gotoxy(4,0); lcd_puts_p(days_names[new_time.weekday]);
		lcd_gotoxy(5,3);
		if(new_time.hour < 10) lcd_puts("0");
		lcd_puti(new_time.hour); lcd_puts(":");
		if(new_time.minute < 10) lcd_puts("0");
		lcd_puti(new_time.minute);
		process_state = next_state;
		break;
	case day:
		boundary_input = 6;
		if(T2 == 0){
			lcd_gotoxy(4,0); lcd_charMode(2);
			if(blink == 1) {
				lcd_puts_p(days_names[input_value]);
				blink = 0;
			}
			else if(blink == 0){
				lcd_clearline(0);
				blink = 1;
			}
			T2 = 25;
		}
		break;
	case hour:
		boundary_input = 23;
		if(T2 == 0){
			lcd_gotoxy(5,3); lcd_charMode(2);
			if(blink == 1) {
				if(input_value < 10) lcd_puts("0");
				lcd_puti(input_value);
				blink = 0;
			}
			else if(blink == 0){
				lcd_puts("  ");
				blink = 1;
			}
			T2 = 25;
		}
		break;
	case minute:
		boundary_input = 59;
		if(T2 == 0){
			lcd_gotoxy(11,3); lcd_charMode(2);
			if(blink == 1) {
				if(input_value < 10) lcd_puts("0");
				lcd_puti(input_value);
				blink = 0;
			}
			else if(blink == 0){
				lcd_puts("  ");
				blink = 1;
			}
			T2 = 25;
		}
		break;
	case end:
		rtc_set_date(&new_time);
		live_time = new_time;
		next_dose = d_find_next_dose();
		process_state = init; next_state = init;
		return 1;
	}
	return 0;
}
uint8_t take_dose(void){
	enum TS {init,first_take,take,wait_for_open,close,wait_end};
	static enum TS take_state;
	static uint8_t doses_taken = 0;

	switch(take_state){
	case init:
		lcd_clrscr();
		lcd_gotoxy(4,2); lcd_puts("Please, press");
		lcd_gotoxy(4,3); lcd_puts("middle button to");
		lcd_gotoxy(4,4); lcd_puts("open compartment");
		if(doses_taken > 0) take_state = take;
		else take_state = first_take;;
		break;
	case first_take:
		doses_taken++;
		take_state = wait_for_open;
		break;
	case take:
		doses_taken++;
		move_one_compartment();
		take_state = wait_for_open;
		break;
	case wait_for_open:
		if(pushed_key == 2){
			servo_enable();
			servo_set_position(10);
			pushed_key = 0;
			T2 = 100;
			take_state = close;
		}
		break;
	case close:
		if(T2 == 0){
		servo_set_position(0);
		lcd_clrscr();
		T2 = 10;
		take_state = wait_end;
		}
		break;
	case wait_end:
		if(T2 == 0){
			servo_disable();
			take_state = init;
			return 1;
		}
		break;
	}
	return 0;
}

void d_main_home_display(void){
	enum DS {display_1,menu,dose_define_fun,view_doses_fun,refill_medicine_fun,change_time_fun,take_dose_fun};
	static enum DS display_state;
	static uint8_t blink;

	if(display_state == display_1){
		lcd_charMode(2);
		lcd_gotoxy(4,0); lcd_puts_p(days_names[live_time.weekday]);
		lcd_gotoxy(5,3);
		if(live_time.hour < 10) lcd_puts("0");
		lcd_puti(live_time.hour); lcd_puts(":");
		if(live_time.minute < 10) lcd_puts("0");
		lcd_puti(live_time.minute);
		lcd_charMode(1);
		lcd_gotoxy(0,5);
		display_state = menu;
	}
	else if(display_state == menu){
		static uint8_t menu_pos = 0, last_menu_pos = 0;
		if(minute_flag == 1){
			lcd_charMode(2);
			lcd_gotoxy(4,0); lcd_puts_p(days_names[live_time.weekday]);
			lcd_gotoxy(5,3);
			if(live_time.hour < 10) lcd_puts("0");
			lcd_puti(live_time.hour); lcd_puts(":");
			if(live_time.minute < 10) lcd_puts("0");
			lcd_puti(live_time.minute);
			lcd_charMode(1);
		}
		if(its_dosetime_flag == 1){
			menu_pos = 5;
			if(T2 == 0){
				if(blink == 1) {
					lcd_invert(1);
					blink = 0;
				}
				else if(blink == 0){
					lcd_invert(0);
					blink = 1;
				}
				T2 = 50;
			}
		}


		switch(pushed_key){
		case 1:
			if(menu_pos > 0) menu_pos--;
			pushed_key = 0;
			break;
		case 2:
			if(menu_pos == 0){
				display_state = dose_define_fun;
				pushed_key = 0;
			}
			else if(menu_pos == 1){
				display_state = view_doses_fun;
				pushed_key = 0;
				lcd_clrscr();
			}
			else if(menu_pos == 2){
				display_state = refill_medicine_fun;
				pushed_key = 0;
			}
			else if(menu_pos == 3){
				display_state = change_time_fun;
				pushed_key = 0;
			}
			else if(menu_pos == 5){
				display_state = take_dose_fun;
				lcd_invert(0);
				pushed_key = 0;
			}
			break;
		case 3:
			if(menu_pos < 4) menu_pos++;
			pushed_key = 0;
			break;
		default:
			break;
		}
		if(last_menu_pos != menu_pos){
			switch(menu_pos){
			case 0:
				lcd_clearline(6);
				lcd_gotoxy(1,6); lcd_puts(">> Define doses <<");
				last_menu_pos = menu_pos;
				break;
			case 1:
				lcd_clearline(6);
				lcd_gotoxy(2,6); lcd_puts(">> View doses <<");
				last_menu_pos = menu_pos;
				break;
			case 2:
				lcd_clearline(6);
				lcd_gotoxy(6,6); lcd_puts(">> Fill <<");
				last_menu_pos = menu_pos;
				break;
			case 3:
				lcd_clearline(6);
				lcd_gotoxy(2,6); lcd_puts(">> Change time <<");
				last_menu_pos = menu_pos;
				break;
			case 4:
				lcd_clearline(6);
				last_menu_pos = menu_pos;
				break;
			case 5:
				lcd_clearline(6);
				lcd_gotoxy(2,6); lcd_puts(">> TAKE DOSE <<");
				last_menu_pos = menu_pos;
				break;
			default:
				break;
			}
		}
	}
	else if(display_state == dose_define_fun){
		if (d_define_doses() == 1) display_state = display_1;
	}
	else if(display_state == view_doses_fun){
		if(d_display_doses() == 1) display_state = display_1;
	}
	else if(display_state == refill_medicine_fun){
		if(d_infill_medicine() == 1) display_state = display_1;
	}
	else if(display_state == change_time_fun){
		if(change_live_time() == 1) display_state = display_1;
	}
	else if(display_state == take_dose_fun){
		if(take_dose() == 1) display_state = display_1;
		its_dosetime_flag = 0;
	}
}
