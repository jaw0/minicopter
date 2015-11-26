/*
  Copyright (c) 2015
  Author: Jeff Weisberg <jaw @ tcp4me.com>
  Created: 2015-Oct-06 00:29 (EDT)
  Function: motors

*/

#include <conf.h>
#include <proc.h>
#include <gpio.h>
#include <pwm.h>
#include <ioctl.h>
#include <error.h>
#include <stm32.h>
#include <userint.h>

#include "board.h"


DEFVAR(short, motor_testspeed, 128, UV_TYPE_US | UV_TYPE_CONFIG, "motor test speed")

void
set_motors(int m1, int m2, int m3, int m4){
    int bv = battery_voltage();

    if( bv ){
        // adjust for battery voltage
        m1 = (m1 * BATT_FULL) / bv;
        m2 = (m2 * BATT_FULL) / bv;
        m3 = (m3 * BATT_FULL) / bv;
        m4 = (m4 * BATT_FULL) / bv;
    }

    if( m1 < 0 ) m1 = 0;
    if( m2 < 0 ) m2 = 0;
    if( m3 < 0 ) m3 = 0;
    if( m4 < 0 ) m4 = 0;

    if( m1 > 1023 ) m1 = 1023;
    if( m2 > 1023 ) m2 = 1023;
    if( m3 > 1023 ) m3 = 1023;
    if( m4 > 1023 ) m4 = 1023;

    pwm_set( HWCF_TIMER_MOTOR_1, m1 );
    pwm_set( HWCF_TIMER_MOTOR_2, m2 );
    pwm_set( HWCF_TIMER_MOTOR_3, m3 );
    pwm_set( HWCF_TIMER_MOTOR_4, m4 );

}

DEFUN(testmotors, "test motors")
{

    printf("M1\n");
    set_motors(motor_testspeed, 0, 0, 0);
    sleep(1);
    set_motors(0,0,0,0);
    sleep(1);

    printf("M2\n");
    set_motors(0, motor_testspeed, 0, 0);
    sleep(1);
    set_motors(0,0,0,0);
    sleep(1);

    printf("M3\n");
    set_motors(0, 0, motor_testspeed, 0);
    sleep(1);
    set_motors(0,0,0,0);
    sleep(1);

    printf("M4\n");
    set_motors(0, 0, 0, motor_testspeed);
    sleep(1);
    set_motors(0,0,0,0);
    sleep(1);

    return 0;
}


DEFUN(testmotors2, "test motors")
{

    int m = atoi(argv[1] );
    set_motors( m, m, m, m );
    sleep(5);
    set_motors(0,0,0,0);
    return 0;

}
