/*
  Copyright (c) 2015
  Author: Jeff Weisberg <jaw @ tcp4me.com>
  Created: 2015-Oct-06 00:34 (EDT)
  Function: board hardware config

*/

#define I2CUNIT			0

#define BATT_FULL 		2600	// "typical" reading for a full battery - ~ 2 * 4096 * 4.2V / 3.3V
#define BATT_WEAK		2172	// weak battery ~ 3.5V
#define BATT_DEAD      		1986	// dead battery ~ 3.2V

#define HWCF_GPIO_BATV		GPIO_A4
#define HWCF_ADC_BATV		ADC_1_4

#define HWCF_GPIO_AUDIO		GPIO_A8
#define HWCF_TIMER_AUDIO	TIMER_1_1

#define HWCF_GPIO_LED_1B	GPIO_B0
#define HWCF_GPIO_LED_1R	GPIO_B1
#define HWCF_GPIO_LED_2B	GPIO_A6
#define HWCF_GPIO_LED_2R	GPIO_A7

#define HWCF_TIMER_LED_1B	TIMER_3_3
#define HWCF_TIMER_LED_1R	TIMER_3_4
#define HWCF_TIMER_LED_2B	TIMER_3_1
#define HWCF_TIMER_LED_2R	TIMER_3_2

#define HWCF_GPIO_MOTOR_1	GPIO_A0
#define HWCF_GPIO_MOTOR_2	GPIO_A1
#define HWCF_GPIO_MOTOR_3	GPIO_A2
#define HWCF_GPIO_MOTOR_4	GPIO_A3

#define HWCF_TIMER_MOTOR_1	TIMER_2_1
#define HWCF_TIMER_MOTOR_2	TIMER_2_2
#define HWCF_TIMER_MOTOR_3	TIMER_2_3
#define HWCF_TIMER_MOTOR_4	TIMER_2_4

