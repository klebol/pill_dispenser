/*
 * StepperMotor.h
 *
 *  Created on: 4 maj 2022
 *      Author: Michal
 */

#ifndef STEPPERMOTOR_H_
#define STEPPERMOTOR_H_


#define Steppin0 (1<<PD6)
#define Steppin1 (1<<PD5)
#define Steppin2 (1<<PD4)
#define Steppin3 (1<<PD3)

#define Krok1 PORTD |= Steppin0; PORTD &= ~(Steppin1 | Steppin2 | Steppin3)
#define Krok2 PORTD |= Steppin1; PORTD &= ~(Steppin0 | Steppin2 | Steppin3)
#define Krok3 PORTD |= Steppin2; PORTD &= ~(Steppin0 | Steppin1 | Steppin3)
#define Krok4 PORTD |= Steppin3; PORTD &= ~(Steppin0 | Steppin1 | Steppin2)
#define StepperOff PORTD &= ~(Steppin1 | Steppin2 | Steppin3 | Steppin0)


int16_t step_pos, step_pos_fu;
uint8_t step_unlock;

void OneStep (uint8_t direction);
void Stepper (void);
void Stepper_init(void);
void move_one_compartment(void);

#endif /* STEPPERMOTOR_H_ */
