################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
LD_SRCS += \
../src/lscript.ld 

C_SRCS += \
../src/main.c \
../src/motor_control.c \
../src/roboclaw.c 

OBJS += \
./src/main.o \
./src/motor_control.o \
./src/roboclaw.o 

C_DEPS += \
./src/main.d \
./src/motor_control.d \
./src/roboclaw.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 gcc compiler'
	arm-none-eabi-gcc -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I/home/ubuntu-z5/proj/motor_uart_00/bare_zynq_proc/export/bare_zynq_proc/sw/bare_zynq_proc/standalone_domain/bspinclude/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


