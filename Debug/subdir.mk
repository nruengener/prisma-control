################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../blmotor.c \
../i2cmaster_V2.c \
../main.c \
../mpu6050.c \
../uart.c 

OBJS += \
./blmotor.o \
./i2cmaster_V2.o \
./main.o \
./mpu6050.o \
./uart.o 

C_DEPS += \
./blmotor.d \
./i2cmaster_V2.d \
./main.d \
./mpu6050.d \
./uart.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


