#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <stdint.h>

/* Open-loop motor commands */
int Motor1_Forward(uint8_t speed);
int Motor1_Backward(uint8_t speed);

int Motor2_Forward(uint8_t speed);
int Motor2_Backward(uint8_t speed);

/* Closed-loop speed commands (encoder counts per second) */
int Motor1_Speed(int32_t speed);
int Motor2_Speed(int32_t speed);

int Motors_Stop(void);

/* Encoder and measured-speed reads */
int Motor1_ReadEncoder(int32_t *encoder);
int Motor2_ReadEncoder(int32_t *encoder);

int Motor1_ReadSpeed(int32_t *speed);
int Motor2_ReadSpeed(int32_t *speed);

#endif
