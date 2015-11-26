/*
  Copyright (c) 2015
  Author: Jeff Weisberg <jaw @ tcp4me.com>
  Created: 2015-Oct-06 00:34 (EDT)
  Function: initialize board

*/

#include <conf.h>
#include <proc.h>
#include <gpio.h>
#include <pwm.h>
#include <adc.h>
#include <spi.h>
#include <i2c.h>
#include <ioctl.h>
#include <error.h>
#include <stm32.h>
#include <userint.h>

#include "board.h"


// T1,T2 => AF(1); T3,T4,T5 => AF(2)
void
board_init(void){

    bootmsg("board hw init:");

    // enable i+d cache, prefetch=off => faster + lower adc noise
    // nb: prefetch=on => more faster, less power, more noise
    FLASH->ACR  |= 0x600;

    // beeper
    gpio_init( HWCF_GPIO_AUDIO,    GPIO_AF(1) | GPIO_SPEED_2MHZ );
    pwm_init(  HWCF_TIMER_AUDIO,   440, 255 );
    pwm_set(   HWCF_TIMER_AUDIO,   0 );

    // LEDs
    gpio_init( HWCF_GPIO_LED_1R,   GPIO_AF(2) | GPIO_SPEED_2MHZ );
    gpio_init( HWCF_GPIO_LED_1B,   GPIO_AF(2) | GPIO_SPEED_2MHZ );
    gpio_init( HWCF_GPIO_LED_2R,   GPIO_AF(2) | GPIO_SPEED_2MHZ );
    gpio_init( HWCF_GPIO_LED_2B,   GPIO_AF(2) | GPIO_SPEED_2MHZ );

    pwm_init(  HWCF_TIMER_LED_1R,  10000, 255 );
    pwm_set(   HWCF_TIMER_LED_1R,  0);
    pwm_set(   HWCF_TIMER_LED_2R,  0);
    pwm_set(   HWCF_TIMER_LED_1B,  0);
    pwm_set(   HWCF_TIMER_LED_2B,  0);

    // motors
    gpio_init( HWCF_GPIO_MOTOR_1,  GPIO_AF(1) | GPIO_SPEED_25MHZ );
    gpio_init( HWCF_GPIO_MOTOR_2,  GPIO_AF(1) | GPIO_SPEED_25MHZ );
    gpio_init( HWCF_GPIO_MOTOR_3,  GPIO_AF(1) | GPIO_SPEED_25MHZ );
    gpio_init( HWCF_GPIO_MOTOR_4,  GPIO_AF(1) | GPIO_SPEED_25MHZ );

    pwm_init(  HWCF_TIMER_MOTOR_1, 40000, 1023 );
    pwm_set(   HWCF_TIMER_MOTOR_1, 0);
    pwm_set(   HWCF_TIMER_MOTOR_2, 0);
    pwm_set(   HWCF_TIMER_MOTOR_3, 0);
    pwm_set(   HWCF_TIMER_MOTOR_4, 0);

    init_power();
    init_imu();
    init_compass();
    init_pressure();

    bootmsg("\n");
}

