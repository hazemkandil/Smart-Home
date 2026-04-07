################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ADC_Program.c \
../APP_Program.c \
../DIO_Program.c \
../EXTI_Program.c \
../KeyPad_Program.c \
../LCD_Program.c \
../Timer1_Program.c \
../USART_Program.c \
../main.c 

OBJS += \
./ADC_Program.o \
./APP_Program.o \
./DIO_Program.o \
./EXTI_Program.o \
./KeyPad_Program.o \
./LCD_Program.o \
./Timer1_Program.o \
./USART_Program.o \
./main.o 

C_DEPS += \
./ADC_Program.d \
./APP_Program.d \
./DIO_Program.d \
./EXTI_Program.d \
./KeyPad_Program.d \
./LCD_Program.d \
./Timer1_Program.d \
./USART_Program.d \
./main.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: AVR Compiler'
	avr-gcc -Wall -g2 -gstabs -O0 -fpack-struct -fshort-enums -ffunction-sections -fdata-sections -std=gnu99 -funsigned-char -funsigned-bitfields -mmcu=atmega32 -DF_CPU=8000000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


