#include "xuartps.h"
#include "xuartps_hw.h"
#include "xparameters.h"
#include "xstatus.h"
#include "xtime_l.h"
#include "stdint.h"
#include "xil_printf.h"

#define UART_DEVICE_ID XPAR_XUARTPS_0_DEVICE_ID

#define ROBOCLAW_ADDRESS  0x80 //as defined in motion studio
#define ROBOCLAW_BAUDRATE 2400 //'', can be set higher in motion studio

//for uart fifo probe
#define UART_STATUS_REG_OFFSET  0x2C
#define UART_TXEMPTY_MASK       0x08
#define UART_TACTIVE_MASK       0x800

static XUartPs Uart0;

// ############################################################
// #### HELPER FUNCS for packing and unpacking 32-bit data ####
// ############################################################
static void PackU32(uint8_t *buffer, uint32_t value)
{
    buffer[0] = (value >> 24) & 0xFF;
    buffer[1] = (value >> 16) & 0xFF;
    buffer[2] = (value >> 8) & 0xFF;
    buffer[3] = value & 0xFF;
}

static uint32_t UnpackU32(const uint8_t *buffer)
{
    return ((uint32_t)buffer[0] << 24) |
           ((uint32_t)buffer[1] << 16) |
           ((uint32_t)buffer[2] << 8) |
           ((uint32_t)buffer[3]);
}

static uint16_t UnpackU16(const uint8_t *buffer)
{
    return ((uint16_t)buffer[0] << 8) |
           ((uint16_t)buffer[1]);
}

static void UART_WaitTransmitDone(void)
{
    uint32_t status;

    do
    {
        status = XUartPs_ReadReg(
            Uart0.Config.BaseAddress,
            UART_STATUS_REG_OFFSET
        );

    } while (
        !(status & UART_TXEMPTY_MASK) ||
        (status & UART_TACTIVE_MASK)
    );
}

// ###########################################
// #### UART SETUP, TRANSMIT, AND RECEIVE ####
// ###########################################
/**
 * Initialize UART0 with RoboClaw baud rate
 * */
int RoboClaw_UART_Init(void)
{
    XUartPs_Config *Config;
    int Status;

    Config = XUartPs_LookupConfig(UART_DEVICE_ID);

    if (Config == NULL)
        return XST_FAILURE;

    Status = XUartPs_CfgInitialize(
        &Uart0,
        Config,
        Config->BaseAddress
    );

    if (Status != XST_SUCCESS)
        return XST_FAILURE;

    Status = XUartPs_SetBaudRate(
        &Uart0,
        ROBOCLAW_BAUDRATE
    );

    if (Status != XST_SUCCESS)
        return XST_FAILURE;

    return XST_SUCCESS;
}

static void UART_CheckPendingData(void)
{
    uint8_t data;
    int count = 0;

    while (XUartPs_Recv(&Uart0, &data, 1) == 1)
    {
        xil_printf("[RX WARNING] Unexpected byte: %02X\r\n", data);
        count++;
    }

    xil_printf("[RX DEBUG] Pending bytes: %d\r\n", count);
}

/**
 * Find CRC16 as required by RoboClaw packet serial control
 * on an array of packet data (bytes)
 *
 * e.g.
 * uint8_t packet[] = {0x80, 0x00, 0x40};
 * uint16_t crc_res = RoboClaw_CRC16(packet, 3);
 *
 * @param data pointer array of bytes of packet data to calculate CRC on
 * @param length how many packets to be checked
 * @return 16-bit CRC result
 */
uint16_t RoboClaw_CRC16(const uint8_t *data, int length)
{
    uint16_t crc = 0;

    for (int i = 0; i < length; i++)
    {
        crc ^= ((uint16_t)data[i] << 8);

        for (int j = 0; j < 8; j++)
        {
            if (crc & 0x8000)
                crc = (crc << 1) ^ 0x1021;
            else
                crc <<= 1;
        }
    }

    return crc;
}


/**
 * Helper funcs to send and recv over UART0
 */
static int UART_SendByte(uint8_t data)
{
    while (XUartPs_Send(&Uart0, &data, 1) != 1)
    {
        // Wait for transmit FIFO space
    }

    return 0;
}

static int UART_ReceiveByte(
    uint8_t *data,
    uint32_t timeout_ms)
{
    XTime start, current;

    XTime_GetTime(&start);

    while (1)
    {
        if (XUartPs_Recv(&Uart0, data, 1) == 1)
            return 0;

        XTime_GetTime(&current);

        if ((current - start) >
            ((XTime)timeout_ms * COUNTS_PER_SECOND / 1000))
        {
            return -1;
        }
    }
}

///**
// * Send a command to the roboclaw with specific packet structure,
// * including performing CRC16 and including on the end
// *
// * @param address default is 0x80, useful if using multiple roboclaws
// * @param command one byte command number
// * @param value e.g. speed for forward move
// */
//int RoboClaw_SendCommand(
//    uint8_t address,
//    uint8_t command,
//    uint8_t value)
//{
//    uint8_t packet[5];
//    uint8_t ack;
//    uint16_t crc;
//
//    packet[0] = address;
//    packet[1] = command;
//    packet[2] = value;
//
//    crc = RoboClaw_CRC16(packet, 3);
//
//    packet[3] = (crc >> 8) & 0xFF;
//    packet[4] = crc & 0xFF;
//
//    for (int i = 0; i < 5; i++)
//    {
//        UART_SendByte(packet[i]);
//    }
//
//    if (UART_ReceiveByte(&ack, 100) != 0)
//        return -1;
//
//    if (ack != 0xFF)
//        return -2;
//
//    return 0;
//}
//
///**
// * Different commands have different value sizes
// * e.g. closed loop speed: [Address, 35, Speed(4 Bytes), CRC16(2 bytes)]
// */
//int RoboClaw_SendCommand_4B(
//    uint8_t address,
//    uint8_t command,
//    int32_t value) //signed value, negative is backwards
//{
//    uint8_t packet[8];
//    uint8_t ack;
//    uint16_t crc;
//
//    packet[0] = address;
//    packet[1] = command;
//
//    // Split signed 32-bit value into big-endian bytes
//    //	--> see user manual pg 66
//    packet[2] = ((uint32_t)value >> 24) & 0xFF;
//    packet[3] = ((uint32_t)value >> 16) & 0xFF;
//    packet[4] = ((uint32_t)value >> 8) & 0xFF;
//    packet[5] = (uint32_t)value & 0xFF;
//
//    // Calculate CRC
//    crc = RoboClaw_CRC16(packet, 6);
//
//    packet[6] = (crc >> 8) & 0xFF;
//    packet[7] = crc & 0xFF;
//
//    // Transmit packet
//    for (int i = 0; i < 8; i++)
//    {
//        UART_SendByte(packet[i]);
//    }
//
//    // Wait for acknowledgment
//    if (UART_ReceiveByte(&ack, 100) != 0)
//        return -1;
//
//    if (ack != 0xFF)
//        return -2;
//
//    return 0;
//}
//
///**
// * Request and read a four byte packet
// * e.g. ask for encoder 1 and read res
// */
//static int RoboClaw_Read4B(
//    uint8_t address,
//    uint8_t command,
//    int32_t *value,
//    uint8_t *status)
//{
//    uint8_t packet[2];
//    uint8_t response[7];
//    uint16_t crc;
//
//    // Build read request
//    packet[0] = address;
//    packet[1] = command;
//
//    // Send request (read commands do not use request CRC)
//    UART_SendByte(packet[0]);
//    UART_SendByte(packet[1]);
//
//    // Receive 4-byte value, status, and CRC
//    for (int i = 0; i < 7; i++)
//    {
//        if (UART_ReceiveByte(&response[i], 100) != 0)
//            return -1;
//    }
//
//    // Calculate CRC over address, command, and response data
//    uint8_t crc_data[7];
//
//    crc_data[0] = address;
//    crc_data[1] = command;
//
//    for (int i = 0; i < 5; i++)
//        crc_data[i + 2] = response[i];
//
//    crc = RoboClaw_CRC16(crc_data, 7);
//
//    // Verify CRC
//    uint16_t received_crc =
//        ((uint16_t)response[5] << 8) |
//        response[6];
//
//    if (crc != received_crc)
//        return -2;
//
//    // Reconstruct signed 32-bit value
//    uint32_t raw_value =
//        ((uint32_t)response[0] << 24) |
//        ((uint32_t)response[1] << 16) |
//        ((uint32_t)response[2] << 8) |
//        ((uint32_t)response[3]);
//
//    *value = (int32_t)raw_value;
//    *status = response[4];
//
//    return 0;
//}

//###########################################################################
//#### GENERIC ROBOCLAW PACKET READ/WRITE to generalize to more commands ####
//###########################################################################

/**
 * Writes a packet of addr, cmd, payload (with payload of specified length)
 * to the RoboClaw UART connection. Waits for an ACK from the RoboClaw.
 */
int RoboClaw_WritePacket(
    uint8_t address,
    uint8_t command,
    const uint8_t *payload,
    uint8_t payload_length)
{
    uint8_t packet[32];
    uint8_t ack;

    if (payload_length > 28)
        return -1;

    packet[0] = address;
    packet[1] = command;

    // Copy payload
    for (int i = 0; i < payload_length; i++)
    {
        packet[i + 2] = payload[i];
    }

    // Calculate CRC over address, command, and payload
    uint16_t crc = RoboClaw_CRC16(
        packet,
        payload_length + 2
    );

    packet[payload_length + 2] = (crc >> 8) & 0xFF;
    packet[payload_length + 3] = crc & 0xFF;

    // Transmit entire packet
    for (int i = 0; i < payload_length + 4; i++)
    {
        if (UART_SendByte(packet[i]) != 0)
            return -2;
    }

    //Wait for end of transmission
    UART_WaitTransmitDone();

    // Wait for ACK
    if (UART_ReceiveByte(&ack, 100) != 0) //timeout
    {
        xil_printf("[ERROR] ACK timeout: command %02X\r\n", command);
        return -3;
    }

    if (ack != 0xFF) //bad ackpple
    {
        xil_printf("[ERROR] Invalid ACK: command %02X, received %02X\r\n",
                   command, ack);
        return -4;
    } else {
    	xil_printf("Valid ACK: command %02X, received %02X\r\n", command, ack);
    }

    return 0;
}

/**
 * Requests and reads a packet from the RoboClaw, where response
 * length is allowed to vary, handled by pointer.
 */
int RoboClaw_ReadPacket(
    uint8_t address,
    uint8_t command,
    uint8_t *response,
    uint8_t response_length)
{
    uint8_t crc_data[32];

    if (response_length > 28)
        return -1;

    // Send read request
    if (UART_SendByte(address) != 0)
        return -2;

    if (UART_SendByte(command) != 0)
        return -2;

    //Wait for end of transmission
    UART_WaitTransmitDone();

    // Include address and command in CRC calculation
    crc_data[0] = address;
    crc_data[1] = command;

    // Receive response payload
    for (int i = 0; i < response_length; i++)
    {
        if (UART_ReceiveByte(&response[i], 100) != 0)
        {
            uint32_t uart_isr = XUartPs_ReadReg(
                Uart0.Config.BaseAddress,
                XUARTPS_ISR_OFFSET
            );

            xil_printf(
                "[RX ERROR] Timeout on payload byte %d of %d\r\n",
                i + 1,
                response_length
            );
            xil_printf("[UART ISR] 0x%08X\r\n", uart_isr);


            return -3;
        }

        crc_data[i + 2] = response[i];
    }

    // Receive CRC high byte
    uint8_t crc_high;
    uint8_t crc_low;

    if (UART_ReceiveByte(&crc_high, 100) != 0)
    {
        uint32_t uart_isr = XUartPs_ReadReg(
            Uart0.Config.BaseAddress,
            XUARTPS_ISR_OFFSET
        );

        xil_printf("[RX ERROR] Timeout on CRC high byte\r\n");
        xil_printf("[UART ISR] 0x%08X\r\n", uart_isr);
        return -3;
    }

    // Receive CRC low byte
    if (UART_ReceiveByte(&crc_low, 500) != 0)
    {
        uint32_t uart_isr = XUartPs_ReadReg(
            Uart0.Config.BaseAddress,
            XUARTPS_ISR_OFFSET
        );

        xil_printf("[RX ERROR] Timeout on CRC low byte\r\n");
        xil_printf("[UART ISR] 0x%08X\r\n", uart_isr);

        return -3;
    }

    uint16_t received_crc =
        ((uint16_t)crc_high << 8) | crc_low;

    uint16_t calculated_crc =
        RoboClaw_CRC16(crc_data, response_length + 2);

    if (received_crc != calculated_crc)
    {
        uint32_t uart_isr = XUartPs_ReadReg(
            Uart0.Config.BaseAddress,
            XUARTPS_ISR_OFFSET
        );

        xil_printf("[CRC ERROR]\r\n");
        xil_printf("Received CRC:   %04X\r\n", received_crc);
        xil_printf("Calculated CRC: %04X\r\n", calculated_crc);
        xil_printf("[UART ISR] 0x%08X\r\n", uart_isr);

        return -4;
    }

    return 0;
}
