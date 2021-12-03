#pragma once

#define DRIVER_0_ADDRES           0
#define DRIVER_0_ENABLE           GPIO_NUM_23   // H= disable motor output

#define VCC_IO_0                  GPIO_NUM_33  // L = reset driver 0, H = driver0 on
#define VCC_IO_1                  GPIO_NUM_25  // L = reset driver 0, H = driver0 on
#define VCC_IO_2                  GPIO_NUM_26  // L = reset driver 0, H = driver0 on
#define VCC_IO_3                  GPIO_NUM_27  // L = reset driver 0, H = driver0 on

#define SW_CTRL                   GPIO_NUM_32  // L = transistor Q3 off -> motor power off, H = all drivers on

#define DRIVERS_UART              UART_NUM_1
#define DRIVERS_UART_TXD          GPIO_NUM_17  // doma 27
#define DRIVERS_UART_RXD          GPIO_NUM_16  // doma 26
#define DRIVERS_UART_BUF_SIZE     256
#define DRIVERS_RX_TIMEOUT        (20 / portTICK_RATE_MS)
#define DRIVERS_UART_START_BYTE   0x05

#define GPIO_OUTPUT_PIN_SEL ((1ULL<<DRIVER_0_ENABLE) | (1ULL<<SW_CTRL) | (1ULL<<VCC_IO_0) )

#define MOTOR_SPEED_COEFICIENT    71608     // 71608 = 1RPS

#define ENCODER_H_LIM_VAL         1000
#define ENCODER_L_LIM_VAL        -1000

// globální proměnné pro pokusy s grafickým rozhraním
volatile int motor_speed = 2 * MOTOR_SPEED_COEFICIENT; 
volatile int motor_load = 0;
volatile int motor_stop_sensitivity = 100;
volatile int potenciometr = 0;
volatile int i_run = 8;
volatile int i_hold = 0;
volatile bool start_stop = false;
volatile uint mot_load[2048];
volatile uint mot_pos[2048];