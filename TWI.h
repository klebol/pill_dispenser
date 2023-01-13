/*
 * TWI.h
 *
 *  Created on: 24 lut 2022
 *      Author: Michal
 */

#ifndef TWI_H_
#define TWI_H_

#define F_I2C			100000UL		//Clock value for I2C
#define PSC_I2C			1				//Prescaler (clock value?) I2C
#define SET_TWBR		(F_CPU/F_I2C-16UL)/(PSC_I2C*2UL)

#define ACK 1
#define NACK 0

extern uint8_t I2C_ErrorCode;		// variable for communication error at twi
									// ckeck it in your code
									// 0 means no error
									// define bits in I2C-ErrorCode:
#define I2C_START		0			// bit 0: timeout start-condition
#define I2C_STOP		1			// bit 0: timeout stop-condition
#define I2C_WRITE		2			// bit 0: timeout write
#define I2C_READ		3			// bit 0: timeout read

void I2C_init(void);
void I2C_start(void);
void I2C_stop(void);
void I2C_write(uint8_t byte);
uint8_t I2C_read(uint8_t ack);
void I2C_begin_comm(uint8_t adr);


#endif /* TWI_H_ */
