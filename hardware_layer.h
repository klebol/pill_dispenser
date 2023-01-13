/*
 * hardware_layer.h
 *
 *  Created on: 29 maj 2022
 *      Author: Michal
 */

#ifndef HARDWARE_LAYER_H_
#define HARDWARE_LAYER_H_

#define key1 (1<<PC0)
#define key2 (1<<PC1)
#define key3 (1<<PC2)
#define stop_switch (1<<PB2)
#define LED (1<<PB5)

#define LED_SW PORTB ^= LED;


volatile uint8_t T1,T2,T3,T_second;
volatile uint8_t second_flag; // data change flag, if = 1, rtc has signaled one second elapse
uint8_t pushed_key, switch_push;


void Reg_init(void);
void Debounce (volatile uint8_t *KPIN, uint8_t kmask,uint16_t rep_wait,uint8_t rep_time, void(*push_proc)(void), void(*rep_proc)(void));

#endif /* HARDWARE_LAYER_H_ */
