#include "roboclaw.h"
#include "motor_control.h"

#define ROBOCLAW_ADDRESS  0x80

/*
 * Convert a signed 32-bit value into the four big-endian payload bytes
 * expected by RoboClaw packet-serial commands.
 */
static void PackI32(uint8_t payload[4], int32_t value)
{
    uint32_t raw = (uint32_t)value;

    payload[0] = (uint8_t)((raw >> 24) & 0xFF);
    payload[1] = (uint8_t)((raw >> 16) & 0xFF);
    payload[2] = (uint8_t)((raw >> 8) & 0xFF);
    payload[3] = (uint8_t)(raw & 0xFF);
}

/*
 * Convert a big-endian four-byte value to int32_t without relying on
 * an implementation-defined unsigned-to-signed conversion.
 */
static int32_t UnpackI32(const uint8_t payload[4])
{
    uint32_t raw =
        ((uint32_t)payload[0] << 24) |
        ((uint32_t)payload[1] << 16) |
        ((uint32_t)payload[2] << 8)  |
        (uint32_t)payload[3];

    if (raw <= INT32_MAX)
        return (int32_t)raw;

    return -1 - (int32_t)(UINT32_MAX - raw);
}

/*
 * Read commands 16-19 return a four-byte value followed by a status byte.
 * RoboClaw_ReadPacket() receives the payload and verifies the CRC.
 */
static int ReadValueAndStatus(
    uint8_t command,
    int32_t *value,
    uint8_t *status)
{
    uint8_t response[5];
    int result;

    if (value == 0 || status == 0)
        return -1;

    result = RoboClaw_ReadPacket(
        ROBOCLAW_ADDRESS,
        command,
        response,
        sizeof(response));

    if (result != 0)
        return result;

    *value = UnpackI32(response);
    *status = response[4];

    return 0;
}


// ######################## MOTOR COMMANDS ########################

// Motor 1 forward: packet-serial command 0, one-byte speed payload.
int Motor1_Forward(uint8_t speed)
{
    return RoboClaw_WritePacket(
        ROBOCLAW_ADDRESS,
        0,
        &speed,
        1);
}

// Motor 1 backward: packet-serial command 1.
int Motor1_Backward(uint8_t speed)
{
    return RoboClaw_WritePacket(
        ROBOCLAW_ADDRESS,
        1,
        &speed,
        1);
}

// Motor 2 forward: packet-serial command 4.
int Motor2_Forward(uint8_t speed)
{
    return RoboClaw_WritePacket(
        ROBOCLAW_ADDRESS,
        4,
        &speed,
        1);
}

// Motor 2 backward: packet-serial command 5.
int Motor2_Backward(uint8_t speed)
{
    return RoboClaw_WritePacket(
        ROBOCLAW_ADDRESS,
        5,
        &speed,
        1);
}

// Motor 1 closed-loop speed: command 35, signed 32-bit speed payload.
int Motor1_Speed(int32_t speed)
{
    uint8_t payload[4];

    PackI32(payload, speed);

    return RoboClaw_WritePacket(
        ROBOCLAW_ADDRESS,
        35,
        payload,
        sizeof(payload));
}

// Motor 2 closed-loop speed: command 36.
int Motor2_Speed(int32_t speed)
{
    uint8_t payload[4];

    PackI32(payload, speed);

    return RoboClaw_WritePacket(
        ROBOCLAW_ADDRESS,
        36,
        payload,
        sizeof(payload));
}

// Stop both motors by commanding zero open-loop speed.
int Motors_Stop(void)
{
    int status1 = Motor1_Forward(0);
    int status2 = Motor2_Forward(0);

    if (status1 != 0)
        return status1;

    if (status2 != 0)
        return status2;

    return 0;
}


// ######################## ENCODER COMMANDS ########################

// Read M1 encoder: command 16 returns count (4 bytes) + status (1 byte).
int Motor1_ReadEncoder(int32_t *encoder)
{
    uint8_t status;
    return ReadValueAndStatus(16, encoder, &status);
}

// Read M2 encoder: command 17.
int Motor2_ReadEncoder(int32_t *encoder)
{
    uint8_t status;
    return ReadValueAndStatus(17, encoder, &status);
}

// Read M1 speed: command 18 returns speed (4 bytes) + status (1 byte).
int Motor1_ReadSpeed(int32_t *speed)
{
    uint8_t status;
    int result = ReadValueAndStatus(18, speed, &status);

    if (result != 0)
        return result;

    /*
     * Preserve the direction handling from the previous implementation:
     * status bit 0 indicates reverse direction for this speed response.
     * If your RoboClaw firmware/manual revision defines this bit differently,
     * adjust this interpretation accordingly.
     */
    if (status & 0x01)
        *speed = -*speed;

    return 0;
}

// Read M2 speed: command 19.
int Motor2_ReadSpeed(int32_t *speed)
{
    uint8_t status;
    int result = ReadValueAndStatus(19, speed, &status);

    if (result != 0)
        return result;

    if (status & 0x01)
        *speed = -*speed;

    return 0;
}
