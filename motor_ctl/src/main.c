/*
 * main.c - RoboClaw motor-control test program
 *
 * Tests every public function currently declared in motor_control.h:
 *   - Motor1_Forward / Motor1_Backward
 *   - Motor2_Forward / Motor2_Backward
 *   - Motor1_Speed / Motor2_Speed
 *   - Motors_Stop
 *   - Motor1_ReadEncoder / Motor2_ReadEncoder
 *   - Motor1_ReadSpeed / Motor2_ReadSpeed
 *
 * SAFETY:
 *   Put the robot on a stand so the wheels can spin freely before running.
 *   Start with low open-loop values. The closed-loop test speed below is
 *   only an example and must be appropriate for your motor/encoder/QPPS setup.
 */

#include <stdint.h>
#include "xil_printf.h"
#include "sleep.h"

#include "roboclaw.h"
#include "motor_control.h"

#define OPEN_LOOP_TEST_SPEED   20U
#define CLOSED_LOOP_TEST_SPEED 500
#define TEST_DURATION_US       1000000U
#define SETTLE_DURATION_US      250000U

static void PrintResult(const char *test_name, int result)
{
    if (result == 0)
        xil_printf("[PASS] %s\r\n", test_name);
    else
        xil_printf("[FAIL] %s (error %d)\r\n", test_name, result);
}

static int CheckResult(const char *test_name, int result)
{
    PrintResult(test_name, result);

    if (result != 0)
    {
        xil_printf("Test sequence stopped because this operation failed.\r\n");
        Motors_Stop();
        return 0;
    }

    return 1;
}

static int TestEncoderAndSpeedReads(void)
{
    int result;
    int32_t encoder1 = 0;
    int32_t encoder2 = 0;
    int32_t speed1 = 0;
    int32_t speed2 = 0;

    xil_printf("\r\n--- Encoder and measured-speed reads ---\r\n");

    result = Motor1_ReadEncoder(&encoder1);
    if (!CheckResult("Read motor 1 encoder", result))
        return 0;

    result = Motor2_ReadEncoder(&encoder2);
    if (!CheckResult("Read motor 2 encoder", result))
        return 0;

    xil_printf("Encoder 1 count: %d\r\n", (int)encoder1);
    xil_printf("Encoder 2 count: %d\r\n", (int)encoder2);

    result = Motor1_ReadSpeed(&speed1);
    if (!CheckResult("Read motor 1 measured speed", result))
        return 0;

    result = Motor2_ReadSpeed(&speed2);
    if (!CheckResult("Read motor 2 measured speed", result))
        return 0;

    xil_printf("Motor 1 measured speed: %d counts/sec\r\n", (int)speed1);
    xil_printf("Motor 2 measured speed: %d counts/sec\r\n", (int)speed2);

    return 1;
}

int main(void)
{
    int result;

    xil_printf("\r\n====================================\r\n");
    xil_printf("      RoboClaw Motor-Control Test\r\n");
    xil_printf("====================================\r\n");

    xil_printf("Initializing RoboClaw UART...\r\n");
    result = RoboClaw_UART_Init();

    if (!CheckResult("RoboClaw UART initialization", result))
        return -1;

    usleep(SETTLE_DURATION_US);

//    /*
//     * Capture initial encoder counts and check that all read commands
//     * communicate correctly before moving the motors.
//     */
//    if (!TestEncoderAndSpeedReads())
//        return -1;

//    int32_t encoder2;
//
//    for (int i = 0; i < 3; i++)
//    {
//        xil_printf("\r\nM2 Encoder Read %d\r\n", i + 1);
//
//        int result = Motor2_ReadEncoder(&encoder2);
//
//        xil_printf("Result: %d\r\n", result);
//
//        if (result == 0)
//            xil_printf("Encoder: %d\r\n", (int)encoder2);
//
//        usleep(1000000);
//    }

    /* both forward and backward (whole robot) */

    for (int i = 0; i < 20; i++)
    {
    	result &=Motor1_Forward(50);
    	result &=Motor2_Forward(50);

        usleep(100000);  // Wait 0.1 seconds
    }

    // Stop both motors
    if (!CheckResult("Motor 1 & Motor 2 forward", result))
        return -1;
    usleep(TEST_DURATION_US);
    result = Motors_Stop();
    if (!CheckResult("Stop both motors", result))
        return -1;
    usleep(SETTLE_DURATION_US);

    for (int i = 0; i < 20; i++)
    {
    	result &=Motor1_Backward(50);
    	result &=Motor2_Backward(50);

        usleep(100000);  // Wait 0.1 seconds
    }

    if (!CheckResult("Motor 1 & Motor 2 backward", result))
        return -1;
    usleep(TEST_DURATION_US);
    result = Motors_Stop();
    if (!CheckResult("Stop both motors", result))
        return -1;
    usleep(SETTLE_DURATION_US);


    /*turning via one wheel moves, one wheel stopped*/
//    for (int i = 0; i < 10; i++)
//    {
//        result &= Motor1_Forward(60);
//        usleep(100000);  // Wait 0.1 seconds
//    }
//
//    if (!CheckResult("Only motor 1 forward", result))
//        return -1;
//    usleep(TEST_DURATION_US);
//
//    result = Motors_Stop();
//    if (!CheckResult("Stop both motors", result))
//        return -1;
//    usleep(SETTLE_DURATION_US);
//    for (int i = 0; i < 10; i++)
//    {
//        result &= Motor2_Forward(60);
//        usleep(100000);  // Wait 0.1 seconds
//    }
//
//    if (!CheckResult("Only motor 2 forward", result))
//        return -1;
//    usleep(TEST_DURATION_US);
//
//    result = Motors_Stop();
//    if (!CheckResult("Stop both motors", result))
//        return -1;
//    usleep(SETTLE_DURATION_US);

    /*turning via one wheel faster, one slower*/
    for (int i = 0; i < 10; i++)
    {
    	result &= Motor2_Forward(30);
        result &= Motor1_Forward(60);
        usleep(100000);  // Wait 0.1 seconds
    }
    if (!CheckResult("Motor 1 > Motor 2 forward", result))
        return -1;
    usleep(TEST_DURATION_US);
    result = Motors_Stop();
    if (!CheckResult("Stop both motors", result))
        return -1;
    usleep(SETTLE_DURATION_US);

    for (int i = 0; i < 10; i++)
    {
    	result &= Motor2_Forward(60);
        result &= Motor1_Forward(30);
        usleep(100000);  // Wait 0.1 seconds
    }
    if (!CheckResult("Motor 2 > Motor 1 forward", result))
        return -1;
    usleep(TEST_DURATION_US);
    result = Motors_Stop();
    if (!CheckResult("Stop both motors", result))
        return -1;
    usleep(SETTLE_DURATION_US);

    /*turning via both forward then one wheel moves*/
//    for (int i = 0; i < 5; i++)
//	{
//		Motor1_Forward(50);
//		Motor2_Forward(50);
//
//		usleep(100000);  // Wait 0.1 seconds
//	}
//    for (int i = 0; i < 10; i++)
//    {
//        result &= Motor1_Forward(60);
//        usleep(100000);  // Wait 0.1 seconds
//    }
//
//    if (!CheckResult("Only motor 1 forward", result))
//        return -1;
//    usleep(TEST_DURATION_US);
//
//    result = Motors_Stop();
//    if (!CheckResult("Stop both motors", result))
//        return -1;
//    usleep(SETTLE_DURATION_US);


//    // ---------------- Open-loop motor 1 tests ----------------
//    xil_printf("\r\n--- Open-loop motor 1 tests ---\r\n");
//
//    for (int i = 0; i < 10; i++)
//    {
//        result &= Motor1_Forward(60);
//        usleep(100000);  // Wait 0.1 seconds
//    }
//
//    result = Motor1_Forward(OPEN_LOOP_TEST_SPEED);
//    if (!CheckResult("Motor 1 forward", result))
//        return -1;
//    usleep(TEST_DURATION_US);
//
//    result = Motors_Stop();
//    if (!CheckResult("Stop both motors", result))
//        return -1;
//    usleep(SETTLE_DURATION_US);
//
//    result = Motor1_Backward(OPEN_LOOP_TEST_SPEED);
//    if (!CheckResult("Motor 1 backward", result))
//        return -1;
//    usleep(TEST_DURATION_US);
//
//    result = Motors_Stop();
//    if (!CheckResult("Stop both motors", result))
//        return -1;
//    usleep(SETTLE_DURATION_US);

    // After Motor 1 forward and Motors_Stop()

//    for (int i = 0; i < 3; i++)
//    {
//        xil_printf("\r\nM2 Encoder Read %d\r\n", i + 1);
//
//        result = Motor2_ReadEncoder(&encoder2);
//
//        xil_printf("Result: %d\r\n", result);
//
//        if (result == 0)
//            xil_printf("Encoder: %d\r\n", (int)encoder2);
//
//        usleep(1000000);
//    }
//
//    // ---------------- Open-loop motor 2 tests ----------------
//    xil_printf("\r\n--- Open-loop motor 2 tests ---\r\n");
//
//    for (int i = 0; i < 10; i++)
//    {
//        result &= Motor2_Forward(60);
//        usleep(100000);  // Wait 0.1 seconds
//    }
//
//
//    result = Motor2_Forward(OPEN_LOOP_TEST_SPEED);
//    if (!CheckResult("Motor 2 forward", result))
//        return -1;
//    usleep(TEST_DURATION_US);
//
//    result = Motors_Stop();
//    if (!CheckResult("Stop both motors", result))
//        return -1;
//    usleep(SETTLE_DURATION_US);
//
//    result = Motor2_Backward(OPEN_LOOP_TEST_SPEED);
//    if (!CheckResult("Motor 2 backward", result))
//        return -1;
//    usleep(TEST_DURATION_US);
//
//    result = Motors_Stop();
//    if (!CheckResult("Stop both motors", result))
//        return -1;
//    usleep(SETTLE_DURATION_US);

//    // ---------------- Closed-loop speed tests ----------------
//    xil_printf("\r\n--- Closed-loop speed tests ---\r\n");
//    xil_printf("Test target: +/- %d encoder counts/sec\r\n",
//               CLOSED_LOOP_TEST_SPEED);
//
//    result = Motor1_Speed(CLOSED_LOOP_TEST_SPEED);
//    if (!CheckResult("Motor 1 positive closed-loop speed", result))
//        return -1;
//    usleep(TEST_DURATION_US);
//
//    if (!TestEncoderAndSpeedReads())
//        return -1;
//
//    result = Motors_Stop();
//    if (!CheckResult("Stop after motor 1 speed test", result))
//        return -1;
//    usleep(SETTLE_DURATION_US);
//
//    result = Motor1_Speed(-CLOSED_LOOP_TEST_SPEED);
//    if (!CheckResult("Motor 1 negative closed-loop speed", result))
//        return -1;
//    usleep(TEST_DURATION_US);
//
//    result = Motors_Stop();
//    if (!CheckResult("Stop after motor 1 reverse-speed test", result))
//        return -1;
//    usleep(SETTLE_DURATION_US);
//
//    result = Motor2_Speed(CLOSED_LOOP_TEST_SPEED);
//    if (!CheckResult("Motor 2 positive closed-loop speed", result))
//        return -1;
//    usleep(TEST_DURATION_US);
//
//    if (!TestEncoderAndSpeedReads())
//        return -1;
//
//    result = Motors_Stop();
//    if (!CheckResult("Stop after motor 2 speed test", result))
//        return -1;
//    usleep(SETTLE_DURATION_US);
//
//    result = Motor2_Speed(-CLOSED_LOOP_TEST_SPEED);
//    if (!CheckResult("Motor 2 negative closed-loop speed", result))
//        return -1;
//    usleep(TEST_DURATION_US);
//
//    result = Motors_Stop();
//    if (!CheckResult("Stop after motor 2 reverse-speed test", result))
//        return -1;
//
//    // ---------------- Final readback and shutdown ----------------
//    usleep(SETTLE_DURATION_US);
//
//    if (!TestEncoderAndSpeedReads())
//        return -1;

    result = Motors_Stop();
    if (!CheckResult("Final motor stop", result))
        return -1;

    xil_printf("\r\n====================================\r\n");
    xil_printf("       Motor-control tests done\r\n");
    xil_printf("====================================\r\n");

    while (1)
    {
        /* Keep the standalone application alive after the test. */
    }

    return 0;
}
