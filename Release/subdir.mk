################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../PCF8563.c \
../StepperMotor.c \
../TWI.c \
../font.c \
../hardware_layer.c \
../lcd.c \
../main.c \
../pill_dispenser.c \
../servo9g.c 

OBJS += \
./PCF8563.o \
./StepperMotor.o \
./TWI.o \
./font.o \
./hardware_layer.o \
./lcd.o \
./main.o \
./pill_dispenser.o \
./servo9g.o 

C_DEPS += \
./PCF8563.d \
./StepperMotor.d \
./TWI.d \
./font.d \
./hardware_layer.d \
./lcd.d \
./main.d \
./pill_dispenser.d \
./servo9g.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: AVR Compiler'
	avr-gcc -I"c:/program files (x86)/atmel/avr tools/avr toolchain/avr/include/avr/iom328p.h" -Wall -Os -fpack-struct -fshort-enums -ffunction-sections -fdata-sections -std=gnu99 -funsigned-char -funsigned-bitfields -mmcu=atmega328p -DF_CPU=16000000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


