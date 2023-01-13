/*
 * TWI.c
 *
 *  Created on: 24 lut 2022
 *      Author: Michal
 */

#include <avr/io.h>
#include "TWI.h"

uint8_t I2C_ErrorCode;

void I2C_init(void){
    // set clock
    switch (PSC_I2C) {
        case 4:
            TWSR = 0x1;
            break;
        case 16:
            TWSR = 0x2;
            break;
        case 64:
            TWSR = 0x3;
            break;
        default:
            TWSR = 0x00;
            break;
    }
    TWBR = (uint8_t)SET_TWBR;
    // enable
    TWCR = (1 << TWEN);
}

void I2C_start(void){
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTA);  // unlock TWI interrupt by setting TWINT Flag to 1 | unlock TWI interface | generate START condition in master mode
	uint16_t timeout = F_CPU/F_I2C*2.0;

	while (!(TWCR & (1<<TWINT))  && timeout !=0){		// wait until TWINT is set to 0 again, that means start was performed successfuly
		timeout--;
		if(timeout == 0){
			I2C_ErrorCode |= (1 << I2C_START);
			return;
		}
	}
}

void I2C_stop(void){
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO); // TWSTO = 1 = generating STOP condition
	uint16_t timeout = F_CPU/F_I2C*2.0;
	while (!(TWCR & (1<<TWSTO))  && timeout !=0){		// wait until TWSTO is set to 0 again, that means stop was performed successfuly
		timeout--;
		if(timeout == 0){
			I2C_ErrorCode |= (1 << I2C_STOP);
			return;
		}
	}
}

void I2C_write(uint8_t byte){
	TWDR = byte;
	TWCR = (1<<TWINT) | (1<<TWEN);
	uint16_t timeout = F_CPU/F_I2C*2.0;
	while (!(TWCR & (1<<TWINT))  && timeout !=0){		// wait until TWSTO is set to 0 again, that means stop was performed successfuly
		timeout--;
		if(timeout == 0){
			I2C_ErrorCode |= (1 << I2C_WRITE);
			return;
		}
	}

}

uint8_t I2C_read(uint8_t ack){
	TWCR = (1<<TWINT) | (1<<TWEN) | (ack<<TWEA);
	uint16_t timeout = F_CPU/F_I2C*2.0;
	while (!(TWCR & (1<<TWINT))  && timeout !=0){		// wait until TWSTO is set to 0 again, that means stop was performed successfuly
		timeout--;
		if(timeout == 0){
			I2C_ErrorCode |= (1 << I2C_READ);
			return 0;
		}
	}
	return TWDR;
}

void I2C_begin_comm(uint8_t adr){
	I2C_start();
	I2C_write(adr);
}



