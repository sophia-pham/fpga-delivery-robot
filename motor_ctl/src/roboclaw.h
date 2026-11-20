#ifndef ROBOCLAW_H
#define ROBOCLAW_H

#include <stdint.h>

int RoboClaw_UART_Init(void);

//int RoboClaw_SendCommand(
//    uint8_t address,
//    uint8_t command,
//    uint8_t value);
//
//int RoboClaw_SendCommand_4B(
//    uint8_t address,
//    uint8_t command,
//    int32_t value); //signed value, negative is backwards

// Generic communication
int RoboClaw_WritePacket(
    uint8_t address,
    uint8_t command,
    const uint8_t *payload,
    uint8_t payload_length);

int RoboClaw_ReadPacket(
    uint8_t address,
    uint8_t command,
    uint8_t *response,
    uint8_t response_length);

#endif
