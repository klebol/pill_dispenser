/*
 * hardware_layer.c
 *
 *  Created on: 29 maj 2022
 *      Author: Michal
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "hardware_layer.h"


ISR(TIMER0_COMPA_vect){
	if(T1) T1--;
	if(T2) T2--;
	if(T3) T3 --;
	if(T_second) T_second--;
	else{
		second_flag = 1;
		T_second = 100;
	}
}

//ISR(INT0_vect){
	//second_flag = 1;

//}

void Reg_init(void){
	TCCR0A |= (1<<WGM01);				//timer CTC mode
	TCCR0B |= (1<<CS02) | (1<<CS00); 	//preskaler 1024
	OCR0A |= 155;						//interrupt 100hz (10ms)
	TIMSK0 |= (1<<OCIE0A);				//interrupt from compare match with OCR0A value
	sei();								//interrupt persmission

	DDRB &=~stop_switch;
	DDRC &=~(key1 | key2 | key3);
	DDRB |= LED;						// LED as an output
	//DDRB |= (1<<PB3);
	PORTC |= key1 | key2 | key3;		//pullup rezistors for keys
	PORTB |= stop_switch; 				//pollup for stop_switch
	PORTB |= LED;						//Led ON

	//EIMSK |= (1<<0);					//external interrupt enable




}

void Debounce (volatile uint8_t *KPIN, uint8_t kmask,uint16_t rep_wait,uint8_t rep_time, void(*push_proc)(void), void(*rep_proc)(void)){
	enum KS {idle,debounce,go_rep,wait_rep,rep};
	static enum KS key_state;
	static uint8_t last_key;
	uint8_t key_press;

	if(last_key && last_key != kmask) return; 							//anti-two button in the same time, if last key is not 0 and its different than last_key value, then return

	key_press = !(*KPIN & kmask);										//check if the key is pressed, store info in key_press variable

	if(key_press && !key_state){										//if button is pressed but was idle before
		key_state = debounce;											//set its state to being in debounce process
		T1 = 5;															//for next 50ms
	}
	else if(key_state){													//if key state isnt idle
		if(key_press && key_state == debounce && !T1){					//if key is still pressed and state is debounce and the timer has finished counting 50ms
			key_state = go_rep;											//set state to go_rep
			last_key = kmask;											//confirm last_key value to current key
			T1 = 3;														//wait another 30ms
		}else
		if(!key_press && key_state > debounce && key_state < rep){		//if key is go_rep or wait_rep (waiting for confirm repeating proccess) and then suddenly has let gone
			if(push_proc) push_proc();									// do function which has been pass via pointer
			key_state = idle;
			last_key = 0;
		}else
		if(key_press && key_state == go_rep && !T1){
			key_state = wait_rep;
			if(!rep_time) rep_time = 20;
			if(!rep_wait) rep_wait = 150;
			T1 = rep_wait;
		}else
		if(key_press && key_state == wait_rep && !T1){
			key_state = rep;
		}else
		if(key_press && key_state == rep && !T1){
			T1 = rep_time;
			if(rep_proc) rep_proc();
		}else
		if(!key_press && key_state == rep){
			key_state = idle;
			last_key = 0;
		}
	}


}
